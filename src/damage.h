#pragma once

#include "distribution.h"
#include "parser/parser.h"

#include <rapidjson/document.h>

#include <string>
#include <map>

class TCreature;

enum class EDamageType {
    Acid,
    Force,
    Cold,
    Fire,
    Lightning,
    Necrosis,
    Piercing,
    Slashing,
    Bludgeoning,
    MagicPiercing,
    MagicSlashing,
    MagicBludgeoning,
    Poison,
    Radiant,
    Thunder,
};

class TDamage {
public:
    TDamage(const rapidjson::Value& json, TParser& parser);

    TDistribution GetDistributionFor(const TCreature* creature) const;

private:
    std::map<EDamageType, TDistribution> damageByType;
};
