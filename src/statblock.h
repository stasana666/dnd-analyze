#pragma once

#include <rapidjson/document.h>

#include <string_view>

#include <vector>
#include <string>

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

    int GetMod(std::string_view sv);
    int GetMod(EStat stat);

    int GetStat(std::string_view sv);
    int GetStat(EStat stat);

    static EStat GetStatByString(std::string_view str);
    static std::string_view GetStringByStat(EStat stat);

private:
    std::array<int, kStatCount> stats;
    std::array<int, kStatCount> modStats;
};

class TSavethrows {

};

class TResources {

};

class TStatblock {
public:
    TStatblock(const rapidjson::Value& json);

    void ReadActions(const rapidjson::Value& json);

    void ReadMeleeAttack(const rapidjson::Value& json);

    void Print();
    std::map<std::string, int> GetVariableMap() const;

    int proficiency;
    TResources resources;
    TSavethrows savethrows;
    TStats stats;
};
