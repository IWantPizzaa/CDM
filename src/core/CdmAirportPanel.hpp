#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

// A local UI snapshot. No credentials, aircraft data or remote-relay flags.
inline std::string BuildCdmAirportPanel(const std::vector<std::string>& configured,
                                       const std::vector<std::string>& local,
                                       const std::vector<std::vector<std::string>>& remote,
                                       const std::string& controller, bool canControl,
                                       bool server, bool nativePanel, bool atfcm, bool bmi,
                                       const std::string& capacityAirport,
                                       const std::vector<std::string>& pending = {}) {
    const auto valid = [](const std::string& airport) {
        return airport.size() == 4 && std::all_of(airport.begin(), airport.end(), [](unsigned char c) {
            return c >= 'A' && c <= 'Z';
        });
    };
    std::map<std::string, bool> airports;
    for (const auto& airport : configured) if (valid(airport)) airports.emplace(airport, false);
    for (const auto& airport : local) if (valid(airport)) airports[airport] = true;
    if (airports.size() > 512) return {};
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartObject(); writer.Key("version"); writer.Int(1);
    writer.Key("can_control"); writer.Bool(canControl);
    writer.Key("native_panel"); writer.Bool(nativePanel);
    writer.Key("atfcm"); writer.Bool(atfcm);
    writer.Key("bmi"); writer.Bool(bmi);
    writer.Key("capacity_airport"); writer.String(capacityAirport.c_str());
    writer.Key("airports"); writer.StartArray();
    for (const auto& [airport, isLocal] : airports) {
        std::string master;
        if (isLocal) master = controller;
        else if (server) for (const auto& entry : remote) {
            if (entry.size() >= 2 && entry[0] == airport) { master = entry[1]; break; }
        }
        if (master.size() > 32 || std::any_of(master.begin(), master.end(), [](unsigned char c) {
            return c < 0x20 || c == 0x7f;
        })) master.clear();
        writer.StartObject(); writer.Key("icao"); writer.String(airport.c_str());
        writer.Key("master"); writer.String(master.c_str());
        writer.Key("local"); writer.Bool(isLocal);
        writer.Key("pending"); writer.Bool(std::find(pending.begin(), pending.end(), airport) != pending.end());
        writer.EndObject();
    }
    writer.EndArray(); writer.EndObject();
    return {buffer.GetString(), buffer.GetSize()};
}
