#include "characteristics.h"

#include <string>
#include <iostream>

namespace {

const std::array<std::string, TStats::kStatCount> kStringByStat = {
    "strength",
    "dexterity",
    "constitution",
    "intelligence",
    "wisdom",
    "charisma",
};

}

TStats::TStats(const rapidjson::Value& json)
{
    for (auto it = json.GetObject().MemberBegin(); it != json.GetObject().MemberEnd(); ++it) {
        stats[static_cast<int>(GetStatByString(it->name.GetString()))] = it->value.GetInt();
    }
    for (int i = 0; i < kStatCount; ++i) {
        modStats[i] = stats[i] / 2 - 5;
    }
}

int TStats::GetMod(std::string_view sv) const
{
    return modStats[static_cast<int>(GetStatByString(sv))];
}

int TStats::GetMod(EStat stat) const
{
    return modStats[static_cast<int>(stat)];
}

int TStats::GetStat(std::string_view sv) const
{
    return stats[static_cast<int>(GetStatByString(sv))];
}

int TStats::GetStat(EStat stat) const
{
    return stats[static_cast<int>(stat)];
}

TStats::EStat TStats::GetStatByString(std::string_view str)
{
    for (int i = 0; i < kStatCount; ++i) {
        if (str == kStringByStat[i]) {
            return static_cast<EStat>(i);
        }
    }
    throw std::logic_error("unknown stat name");
}

const std::string& TStats::GetStringByStat(EStat stat)
{
    return kStringByStat[static_cast<int>(stat)];
}

void TSaveThrows::Init(const rapidjson::Value& json, TParser& parser)
{
    for (int i = 0; i < values.size(); ++i) {
        auto name = kStringByStat[i];
        if (json.HasMember(name.c_str())) {
            values[i] = parser.Parse(json[name.c_str()].GetString());
        } else {
            parser.Parse("mod_" + name);
        }
    }
}
