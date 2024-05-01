#pragma once

#include "dices.h"
#include "creature.h"
#include "statblock.h"
#include <memory>

enum class ETerrainType {
    Normal,
    Difficult,
    Hazardous,
    Uneven,
    Inclines,
    Wall,
};

class TBattleField {
public:
    TBattleField(int width, int height, int randSeed);

    void AddCreature(std::shared_ptr<TStatblock> statblock);

    void RunBattle();

private:
    void RunRound();
    void Step();

    TDices dices;
    std::vector<std::vector<ETerrainType>> terrain;
    std::vector<TCreature> creatures;
};
