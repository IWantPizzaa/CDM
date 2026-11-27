#include "core/CdmAirportPanel.hpp"
#include "rapidjson/document.h"
#include <iostream>

int main() {
    int failures = 0;
    const auto check = [&](bool value, const char* name) {
        if (!value) { ++failures; std::cerr << name << '\n'; }
    };
    const auto text = BuildCdmAirportPanel({"LFPG", "LFPO", "LFPG", "INVALID"}, {"LFOB"},
        {{"LFPO", "LFPO_DEL"}, {"LFPG"}}, "LFOB_TWR", true, true, false, true, true, "LFPG", {"LFPG"});
    rapidjson::Document panel;
    panel.Parse(text.c_str());
    check(!panel.HasParseError() && panel.IsObject(), "panel is valid JSON");
    check(panel["version"].GetInt() == 1 && panel["airports"].Size() == 3, "configured/local airport union is bounded and deduplicated");
    check(panel["airports"][0]["local"].GetBool() && std::string(panel["airports"][0]["master"].GetString()) == "LFOB_TWR",
        "local Master is distinguished from server ownership");
    check(!panel["airports"][1]["local"].GetBool() && panel["airports"][1]["master"].GetStringLength() == 0,
        "missing or malformed remote owner stays unassigned");
    check(panel["airports"][1]["pending"].GetBool(), "pending changes in the original panel are published to vSMR");
    check(std::string(panel["airports"][2]["master"].GetString()) == "LFPO_DEL", "other-controller Master is published");
    check(!panel["native_panel"].GetBool() && panel["atfcm"].GetBool() && panel["bmi"].GetBool() &&
        std::string(panel["capacity_airport"].GetString()) == "LFPG", "floating panels remain independent of the original panel");
    const auto offline = BuildCdmAirportPanel({"LFPG"}, {}, {{"LFPG", "LFPG_DEL"}}, "", false, false, false, false, false, "");
    panel.Parse(offline.c_str());
    check(!panel["can_control"].GetBool() && panel["airports"][0]["master"].GetStringLength() == 0,
        "offline/local-only panel does not advertise stale server ownership");
    std::vector<std::string> many;
    for (char a = 'A'; a <= 'Z'; ++a) for (char b = 'A'; b <= 'Z'; ++b) many.push_back(std::string("LF") + a + b);
    check(BuildCdmAirportPanel(many, {}, {}, "", false, false, false, false, false, "").empty(), "oversized snapshots fail closed");
    return failures ? 1 : 0;
}
