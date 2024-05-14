#include "statblock.h"

#include "parser/parser.h"

#include "damage.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

TStatblock::TStatblock(const rapidjson::Value& json)
    : stats(json["stats"])
    , level(json["level"].GetInt())
    , proficiency(json["proficiency"].GetInt())
    , resources(json["resources"])
    , maxHp(json["max_hp"].GetInt())
    , armourClass(json["armour_class"].GetInt())
{
    const auto& varMap = GetVariableMap();
    TParser parser(varMap);
    savethrows.Init(json["savethrows"], parser);
    ReadActions(parser, json["actions"]);
}

void TStatblock::ReadActions(TParser& parser, const rapidjson::Value& json)
{
    for (const auto& action : json.GetArray()) {
        std::string_view actionType = action["type"].GetString();
        if (actionType == "melee_attack") {
            actions.emplace_back(std::make_unique<TMeleeAttack>(parser, action));
        }
        if (actionType == "range_attack") {
            actions.emplace_back(std::make_unique<TRangeAttack>(parser, action));
        }
        if (actionType == "spell_target_self") {
            actions.emplace_back(std::make_unique<TSpellTargetSelf>(parser, action));
        }
        if (actionType == "spell_target_ally") {
            actions.emplace_back(std::make_unique<TSpellTargetAlly>(parser, action));
        }
        if (actionType == "spell_target_enemy") {
            actions.emplace_back(std::make_unique<TSpellTargetEnemy>(parser, action));
        }
        if (actionType == "spell_area_enemy") {
            actions.emplace_back(std::make_unique<TSpellAreaEnemy>(parser, action));
        }
    }
}

std::map<std::string, int> TStatblock::GetVariableMap() const
{
    std::map<std::string, int> variables;
    variables["proficiency"] = proficiency;
    variables["level"] = level;

    for (int i = 0; i < TStats::kStatCount; ++i) {
        TStats::EStat stat = static_cast<TStats::EStat>(i);
        std::string statName = TStats::GetStringByStat(stat);
        variables[statName] = stats.GetStat(stat);
        variables["mod_" + statName] = stats.GetMod(stat);
    }

    return variables;
}

void TStatblock::Print()
{
    for (int i = 0; i < 6; ++i) {
        std::cerr
            << TStats::GetStringByStat(static_cast<TStats::EStat>(i))
            << ": "
            << stats.GetMod(static_cast<TStats::EStat>(i))
            << std::endl;
    }
    std::cerr << "proficiency: " << proficiency << std::endl;
}
