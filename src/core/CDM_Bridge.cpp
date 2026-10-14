#include "CDMSingle.hpp"
#include "src/core/CDMGlobals.hpp"

#define ESB_CLIENT_SHIM
#include <cctype>
#include <cstdint>
#include <unordered_set>

#include "esbridge.h"

namespace {
enum CdmBridgeField {
    BRIDGE_TOBT,
    BRIDGE_TSAT,
    BRIDGE_TTOT,
    BRIDGE_CTOT,
    BRIDGE_TSAC,
    BRIDGE_ASRT,
    BRIDGE_ASAT,
    BRIDGE_DEICE,
    BRIDGE_TOBT_SET_BY,
    BRIDGE_FLOW_RESTRICTION,
    BRIDGE_ECFMP_RESTRICTION,
    BRIDGE_MANUAL_CTOT,
    BRIDGE_FIELD_COUNT
};

constexpr uint32_t kSyncedDense = ESB_F_SYNC | ESB_F_DENSE;

const ESB_FieldDecl kCdmBridgeFields[] = {
    {"tobt", ESB_T_I64, ESB_SCOPE_AIRCRAFT, kSyncedDense, 0, "Target off-block time, minutes since midnight UTC"},
    {"tsat", ESB_T_I64, ESB_SCOPE_AIRCRAFT, kSyncedDense, 0,
     "Target start-up approval time, minutes since midnight UTC"},
    {"ttot", ESB_T_I64, ESB_SCOPE_AIRCRAFT, kSyncedDense, 0, "Target take-off time, minutes since midnight UTC"},
    {"ctot", ESB_T_I64, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 0, "Calculated take-off time, minutes since midnight UTC"},
    {"tsac", ESB_T_I64, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 0,
     "Communicated target start-up approval time, minutes since midnight UTC"},
    {"asrt", ESB_T_I64, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 0, "Actual start-up request time, minutes since midnight UTC"},
    {"asat", ESB_T_I64, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 0, "Actual start-up approval time, minutes since midnight UTC"},
    {"deice", ESB_T_STR, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 32, "CDM de-icing state or remote de-icing pad identifier"},
    {"tobt_set_by", ESB_T_STR, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 16, "Source that last set TOBT, as reported by CDM"},
    {"flow_restriction", ESB_T_STR, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 512,
     "Flow-restriction reason associated with the CDM slot"},
    {"ecfmp_restriction", ESB_T_STR, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 64,
     "ECFMP restriction identifier associated with the CDM slot"},
    {"manual_ctot", ESB_T_BOOL, ESB_SCOPE_AIRCRAFT, ESB_F_SYNC, 0, "True when the CTOT was assigned manually"},
};

ESB_Provider* bridgeProvider = nullptr;
ESB_FieldId bridgeFieldIds[BRIDGE_FIELD_COUNT] = {};
bool bridgeRegistrationConflict = false;
unordered_set<string> bridgePublishedCallsigns;

bool bridgeTimeToMinutes(const string& text, int64_t& result) {
    if (text.empty()) return false;

    string digits;
    digits.reserve(text.size());
    for (unsigned char character : text) {
        if (isdigit(character)) {
            digits.push_back(static_cast<char>(character));
        } else if (character != ':' && !isspace(character)) {
            return false;
        }
    }

    if (digits.empty() || digits.size() > 6) return false;
    if (digits.size() < 4) digits.insert(0, 4 - digits.size(), '0');

    const int hours = (digits[0] - '0') * 10 + (digits[1] - '0');
    const int minutes = (digits[2] - '0') * 10 + (digits[3] - '0');
    if (minutes > 59 || hours > 24 || (hours == 24 && minutes != 0)) return false;

    result = hours == 24 ? 0 : hours * 60 + minutes;
    return true;
}

void bridgeSetTime(ESB_Aircraft aircraft, CdmBridgeField field, const string& text) {
    int64_t minutes = 0;
    if (!bridgeTimeToMinutes(text, minutes)) {
        esb_api->clear_ac(bridgeProvider, aircraft, bridgeFieldIds[field]);
        return;
    }

    ESB_Value value = ESB_I64(minutes);
    esb_api->set_ac(bridgeProvider, aircraft, bridgeFieldIds[field], &value);
}

void bridgeSetString(ESB_Aircraft aircraft, CdmBridgeField field, const string& text) {
    if (text.empty()) {
        esb_api->clear_ac(bridgeProvider, aircraft, bridgeFieldIds[field]);
        return;
    }

    ESB_Value value = ESB_Str(text.c_str());
    esb_api->set_ac(bridgeProvider, aircraft, bridgeFieldIds[field], &value);
}

void bridgeClearAircraft(ESB_Aircraft aircraft) {
    for (ESB_FieldId field : bridgeFieldIds) {
        if (field != ESB_FIELD_NONE) esb_api->clear_ac(bridgeProvider, aircraft, field);
    }
}

string findCallsignValue(const vector<vector<string>>& values, const string& callsign) {
    for (const auto& value : values) {
        if (value.size() >= 2 && value[0] == callsign) return value[1];
    }
    return "";
}

string findAsat(const string& callsign) {
    const string prefix = callsign + ",";
    for (const string& value : asatList) {
        if (value.rfind(prefix, 0) == 0) return value.substr(prefix.size());
    }
    return "";
}
}  // namespace

void CDM::updateBridge() {
    const ESB_Api_v1* api = ESB_Attach();
    if (!api || bridgeRegistrationConflict) return;

    if (!bridgeProvider) {
        ESB_ProviderDecl declaration = {};
        declaration.struct_size = sizeof declaration;
        declaration.provider_id = "com.viffsys.cdm";
        declaration.schema_major = 1;
        declaration.schema_minor = 0;
        declaration.display_name = "CDM Plugin";
        declaration.contact = "https://github.com/IWantPizzaa/CDM";
        declaration.fields = kCdmBridgeFields;
        declaration.field_count = static_cast<uint32_t>(BRIDGE_FIELD_COUNT);
        declaration.module = ESB_SelfModule();

        const ESB_Status status = api->register_provider(&declaration, &bridgeProvider);
        if (status == ESB_E_PROVIDER_TAKEN) {
            bridgeRegistrationConflict = true;
            addLogLine("ERROR: EuroScope Plugin Bridge provider id com.viffsys.cdm is already registered");
            return;
        }
        if (status != ESB_OK) return;

        for (uint32_t index = 0; index < BRIDGE_FIELD_COUNT; ++index) {
            if (api->own_field(bridgeProvider, kCdmBridgeFields[index].name, &bridgeFieldIds[index]) != ESB_OK) {
                api->unregister_provider(bridgeProvider);
                bridgeProvider = nullptr;
                fill(begin(bridgeFieldIds), end(bridgeFieldIds), ESB_FIELD_NONE);
                addLogLine("ERROR: Failed to resolve a CDM EuroScope Plugin Bridge field");
                return;
            }
        }
    }

    unordered_set<string> publishedThisTick;
    for (const Plane& plane : slotList) {
        if (plane.callsign.empty()) continue;

        ESB_Aircraft aircraft = ESB_AIRCRAFT_NONE;
        if (api->aircraft(plane.callsign.c_str(), &aircraft) != ESB_OK) continue;
        publishedThisTick.insert(plane.callsign);

        CFlightPlan flightPlan = FlightPlanSelect(plane.callsign.c_str());
        const string tobt = flightPlan.IsValid() ? getFlightStripInfo(flightPlan, 2) : "";
        const string tsac = flightPlan.IsValid() ? getFlightStripInfo(flightPlan, 1) : "";
        const string asrt = flightPlan.IsValid() ? getFlightStripInfo(flightPlan, 0) : "";
        const string tobtSetBy = flightPlan.IsValid() ? getFlightStripInfo(flightPlan, 9) : "";

        bridgeSetTime(aircraft, BRIDGE_TOBT, tobt);
        bridgeSetTime(aircraft, BRIDGE_TSAT, plane.tsat);
        bridgeSetTime(aircraft, BRIDGE_TTOT, plane.ttot);
        bridgeSetTime(aircraft, BRIDGE_CTOT, plane.ctot);
        bridgeSetTime(aircraft, BRIDGE_TSAC, tsac == "9999" ? "" : tsac);
        bridgeSetTime(aircraft, BRIDGE_ASRT, asrt);
        bridgeSetTime(aircraft, BRIDGE_ASAT, findAsat(plane.callsign));
        bridgeSetString(aircraft, BRIDGE_DEICE, findCallsignValue(deiceList, plane.callsign));
        bridgeSetString(aircraft, BRIDGE_TOBT_SET_BY, tobtSetBy);
        bridgeSetString(aircraft, BRIDGE_FLOW_RESTRICTION, plane.flowReason);
        bridgeSetString(aircraft, BRIDGE_ECFMP_RESTRICTION, "");

        ESB_Value manualCtot = ESB_Bool(plane.hasManualCtot);
        api->set_ac(bridgeProvider, aircraft, bridgeFieldIds[BRIDGE_MANUAL_CTOT], &manualCtot);
    }

    for (const string& callsign : bridgePublishedCallsigns) {
        if (publishedThisTick.find(callsign) != publishedThisTick.end()) continue;

        ESB_Aircraft aircraft = ESB_AIRCRAFT_NONE;
        if (api->aircraft(callsign.c_str(), &aircraft) == ESB_OK) bridgeClearAircraft(aircraft);
    }
    bridgePublishedCallsigns = move(publishedThisTick);
}

void CDM::shutdownBridge() {
    if (esb_api && bridgeProvider) esb_api->unregister_provider(bridgeProvider);
    bridgeProvider = nullptr;
    fill(begin(bridgeFieldIds), end(bridgeFieldIds), ESB_FIELD_NONE);
    bridgePublishedCallsigns.clear();
}
