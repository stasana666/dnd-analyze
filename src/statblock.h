#pragma once

#include "resources.h"
#include "action.h"
#include "damage.h"

#include <rapidjson/document.h>

#include <unordered_set>
#include <string_view>
#include <vector>
#include <string>
#include <map>

class TStatblock {
public:
    TStatblock(const rapidjson::Value& json);

    void ReadActions(TParser& parser, const rapidjson::Value& json);

    void Print();
    std::map<std::string, int> GetVariableMap() const;

    int level;
    int proficiency;
    int maxHp;
    int armourClass;
    TResources resources;
    TStats stats;
    TSaveThrows savethrows;

    std::unordered_set<EDamageType> immunes;
    std::unordered_set<EDamageType> resists;

    std::vector<std::unique_ptr<TBaseAction>> actions;
};
