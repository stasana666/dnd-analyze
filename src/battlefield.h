#pragma once

#include "creature.h"
#include <memory>

class TBattleField {
public:
    TBattleField(std::vector<TCreature> creatures);
    TBattleField(const TBattleField& battleField);

    void OnRoundStart();
    void OnTurnEnd();
    void OnTurnStart();

    double GetEstimate() const;

    int ActiveCreatureId() const { return activeCreature - creatures.begin(); }

    std::vector<TCreature> creatures;
    std::vector<TCreature>::iterator activeCreature;
    int round = 0;
    std::array<int, 2> originalTeamSize;
    bool turnStart = false;
};

// Для различных ситуаций, когда нужно рассматривать несколько исходов одного действия
// Например получило ли цель достаточно урона, чтобы умереть или нет
using TParallelWorlds = std::vector<std::pair<double, TBattleField>>;

class TEncounter {
public:
    TEncounter(int width, int height);

    void AddCreature(std::shared_ptr<const TStatblock> statblock, int x, int y, int team);

    double GetWinProbability();

    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TMeleeAttack& action, bool& hasAction);
    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TRangeAttack& action, bool& hasAction);
    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TSpellTargetSelf& action, bool& hasAction);
    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TSpellTargetAlly& action, bool& hasAction);
    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TSpellTargetEnemy& action, bool& hasAction);
    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TSpellAreaEnemy& action, bool& hasAction);

    std::vector<TParallelWorlds> MakeAction(const TBattleField& battleField, const TBaseAction* actionPtr, bool& hasAction);

private:
    TParallelWorlds OnTurnStart(TBattleField battleField) const;

    enum class EMode {
        FindBestMove,
        FindAnswer
    };

    struct TConfig {
        double alpha;
        double beta;
        double weightOfTimeline;
        EMode mode;
        int depth;
    };

    double GetWinProbability(TBattleField battleField, TConfig config, std::string space);
    void RollInitiative();

    int width;
    int height;
    std::vector<TCreature> creatures;
};
