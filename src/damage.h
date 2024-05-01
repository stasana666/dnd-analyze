#pragma once

#include "distribution.h"

#include <string>
#include <map>

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
    TDamage(const std::string& expression);

private:
    std::map<EDamageType, TDistribution> damageByType;
};
