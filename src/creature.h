#pragma once

#include "statblock.h"

class TCreature {
public:
    TCreature(const TStatblock& statblock);

private:
    const TStatblock* parent;
    int current_hitpoints;
};
