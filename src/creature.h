#pragma once

#include "statblock.h"
#include "status.h"

struct TPosition {
    int x;
    int y;
};

class TCreature {
public:
    TCreature(std::shared_ptr<const TStatblock> statblock);

    bool IsImmune(EDamageType type) const { return parent->immunes.count(type); }
    bool IsResist(EDamageType type) const { return parent->resists.count(type); }

    double GetEstimate() const;

    std::shared_ptr<const TStatblock> parent;
    TPosition position;
    TResources resources;
    TDistribution hitpoints;
    std::vector<EStatus> status;
    int speed = 6;
    int team;
};
