#include "creature.h"

TCreature::TCreature(const TStatblock& statblock)
    : parent(&statblock)
    , position({0, 0})
    , resources(statblock.resources)
    , hitpoints(statblock.maxHp)
{
}
