#include "damage.h"

#include "creature.h"

#include <map>

namespace {

std::map<EDamageType, std::string> damageTypeToStr {
    { EDamageType::Acid, "acid" },
    { EDamageType::Force, "force" },
    { EDamageType::Cold, "cold" },
    { EDamageType::Fire, "fire" },
    { EDamageType::Lightning, "lightning" },
    { EDamageType::Necrosis, "necrosis" },
    { EDamageType::Piercing, "piercing" },
    { EDamageType::Slashing, "slashing" },
    { EDamageType::Bludgeoning, "bludgeoning" },
    { EDamageType::MagicPiercing, "magic_piercing" },
    { EDamageType::MagicSlashing, "magic_slashing" },
    { EDamageType::MagicBludgeoning, "magic_bludgeoning" },
    { EDamageType::Poison, "poison" },
    { EDamageType::Radiant, "radiant" },
    { EDamageType::Thunder, "thunder" },
};

}

TDistribution TDamage::GetDistributionFor(const TCreature* creature) const
{
    TDistribution resultDamage(0);
    for (auto& [type, damage] : damageByType) {
        if (creature->IsImmune(type)) {
            continue;
        }
        if (creature->IsResist(type)) {
            resultDamage += (damage / 2);
            continue;
        }
        resultDamage += damage;
    }
    return resultDamage;
}

TDamage::TDamage(const rapidjson::Value& json, TParser& parser)
{
    for (auto& [type, name] : damageTypeToStr) {
        if (json.HasMember(name.c_str())) {
            damageByType.insert(std::make_pair(type, parser.ParseDistribution(json[name.c_str()].GetString())));
        }
    }
}
