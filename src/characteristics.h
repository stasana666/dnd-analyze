#pragma once

#include "parser/parser.h"

#include <rapidjson/document.h>

#include <array>

class TStats {
public:
    static constexpr int kStatCount = 6;

    enum class EStat {
        Strength,
        Dexterity,
        Constitution,
        Intelligence,
        Wisdom,
        Charisma,
    };

    TStats(const rapidjson::Value& json);

    int GetMod(std::string_view sv) const;
    int GetMod(EStat stat) const;

    int GetStat(std::string_view sv) const;
    int GetStat(EStat stat) const;

    static EStat GetStatByString(std::string_view str);
    static const std::string& GetStringByStat(EStat stat);

private:
    std::array<int, kStatCount> stats;
    std::array<int, kStatCount> modStats;
};

class TSaveThrows {
public:
    void Init(const rapidjson::Value& json, TParser& parser);

    int operator[] (TStats::EStat stat) const { return values[static_cast<int>(stat)]; }

private:
    std::array<int, TStats::kStatCount> values;
};
