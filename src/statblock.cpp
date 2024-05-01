#include "statblock.h"

#include "parser/tokenizer.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace {

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

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

int TStats::GetMod(std::string_view sv)
{
    return modStats[static_cast<int>(GetStatByString(sv))];
}

int TStats::GetMod(EStat stat)
{
    return modStats[static_cast<int>(stat)];
}

int TStats::GetStat(std::string_view sv)
{
    return stats[static_cast<int>(GetStatByString(sv))];
}

int TStats::GetStat(EStat stat)
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

std::string_view TStats::GetStringByStat(EStat stat)
{
    return kStringByStat[static_cast<int>(stat)];
}

TStatblock::TStatblock(const rapidjson::Value& json)
    : stats(json["stats"])
    , proficiency(json["proficiency"].GetInt())
{
    Print();
    ReadActions(json["actions"]);
}

void TStatblock::ReadActions(const rapidjson::Value& json)
{
    for (const auto& action : json.GetArray()) {
        std::string_view actionType = action["type"].GetString();
        if (actionType == "melee_attack") {
            ReadMeleeAttack(action);
        }
    }
}

void TStatblock::ReadMeleeAttack(const rapidjson::Value& json)
{
    int atkBonus = 0;
    {
        TTokenizer tokenizer(json["attack"].GetString());
        while (!tokenizer.IsEnd()) {
            NToken::TToken token = tokenizer.Next();
            std::visit(overloaded{
                [&](NToken::EOperator op) {

                },
                [&](NToken::TNumber number) {

                },
                [&](NToken::TString str) {

                }
            }, token);
        }
    }
}

std::map<std::string, int> TStatblock::GetVariableMap() const
{
    std::map<std::string, int> variables;
    variables["proficiency"] = proficiency;

    for (int i = 0; i < TStats::kStatCount; ++i) {
        std::string_view statName = TStats::GetStringByStat(static_cast<TStats::EStat>(i));
        variables[statName] = 
    }
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
