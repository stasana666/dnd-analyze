#include "battlefield.h"

#include <algorithm>
#include <iostream>
#include <deque>
#include <cmath>
#include <set>

namespace {

constexpr int kDepth = 11;

}

TBattleField::TBattleField(std::vector<TCreature> creatures_)
    : creatures(creatures_)
    , activeCreature(this->creatures.end())
    , round(0)
    , originalTeamSize({0, 0})
{
    for (auto& creature : creatures) {
        ++originalTeamSize[creature.team];
    }
}

TBattleField::TBattleField(const TBattleField& battleField)
    : creatures(battleField.creatures)
    , activeCreature(creatures.begin() + (battleField.activeCreature - battleField.creatures.begin()))
    , round(battleField.round)
    , originalTeamSize(battleField.originalTeamSize)
{
}

void TBattleField::OnRoundStart()
{
    ++round;
    activeCreature = creatures.begin();
}

void TBattleField::OnTurnEnd()
{
    ++activeCreature;
    if (activeCreature == creatures.end()) {
        OnRoundStart();
    }
    OnTurnStart();
}

void TBattleField::OnTurnStart()
{
    activeCreature->resources.OnTurnStart();
    activeCreature->speed = 6;
}

double TBattleField::GetEstimate() const
{
    std::array<double, 2> result = {0., 0.};
    for (auto& creature : creatures) {
        result[creature.team] += creature.GetEstimate();
    }
    result[0] /= originalTeamSize[0];
    result[1] /= originalTeamSize[1];
    return 1. / (1. + std::exp(2 * (result[1] - result[0])));
}

TEncounter::TEncounter(int width, int height, int randSeed)
    : rng(randSeed)
    , width(width)
    , height(height)
{
}

void TEncounter::AddCreature(const TStatblock& statblock, int x, int y, int team)
{
    creatures.emplace_back(statblock);
    creatures.back().position = TPosition{x, y};
    creatures.back().team = team;
}

double TEncounter::GetWinProbability()
{
    TBattleField battleField(creatures);
    battleField.OnRoundStart();
    return GetWinProbability(
        battleField,
        TConfig{
            .alpha = 0.,
            .beta = 1.,
            .weightOfTimeline = 1.,
            .mode = EMode::FindAnswer,
            .depth = kDepth
        },
        ""
    );
}

double TEncounter::GetWinProbability(TBattleField battleField, TConfig config, std::string space)
{
    if (config.weightOfTimeline < 1e-3) {
        return battleField.GetEstimate();
    }

    const int CurrentTeam = battleField.activeCreature->team;
    std::array<int, 2> teamsSize{0, 0};
    for (auto& creature : battleField.creatures) {
        ++teamsSize[creature.team];
    }
    if (teamsSize[0] == 0) {
        return 0;
    }
    if (teamsSize[1] == 0) {
        return 1;
    }

    if (config.mode == EMode::FindBestMove && config.depth == 0) {
        return battleField.GetEstimate();
    }

    double result = CurrentTeam;
    TParallelWorlds* bestMove = nullptr;
    auto updateResult = [&](double localRes, TParallelWorlds* move) {
        if (CurrentTeam == 0) {
            if (localRes > result) {
                bestMove = move;
            }
            config.alpha = std::max(config.alpha, localRes);
            result = std::max(result, localRes);
        } else if (CurrentTeam == 1) {
            if (localRes < result) {
                bestMove = move;
            }
            config.beta = std::min(config.beta, localRes);
            result = std::min(result, localRes);
        } else {
            throw std::logic_error("unexpected team");
        }
        if (config.alpha >= config.beta - 1e-3) {
            return true;
        }
        return false;
    };

    bool hasNotOptionalAction = false;

    std::vector<TParallelWorlds> multiverse;
    for (const std::unique_ptr<TBaseAction>& actionPtr : battleField.activeCreature->parent->actions) {
        std::vector<TParallelWorlds> worldsAfterThisAction = MakeAction(battleField, actionPtr.get(), hasNotOptionalAction);
        for (auto& world : worldsAfterThisAction) {
            multiverse.emplace_back(world);
        }
    }

    for (TParallelWorlds& worlds : multiverse) {
        double fullResult = 0;
        for (auto& [prob, world] : worlds) {
            fullResult += prob * GetWinProbability(
                world,
                TConfig{
                    .alpha = config.alpha,
                    .beta = config.beta,
                    .weightOfTimeline = config.weightOfTimeline * prob,
                    .mode = EMode::FindBestMove,
                    .depth = config.depth - 1
                },
                space + "  "
            );
        }
        if (updateResult(fullResult, &worlds)) {
            // Нашли ожидаемый лучший ход - сделаем его и найдем ответ для новой позиции
            if (config.mode == EMode::FindAnswer) {
                if (bestMove == nullptr) {
                    std::cerr << "nullptr 1" << std::endl;
                }
                std::cerr << "kek" << std::endl;
                result = 0;
                for (auto& [prob, world] : (*bestMove)) {
                    result += prob * GetWinProbability(
                        world,
                        TConfig{
                            .alpha = 0.,
                            .beta = 1.,
                            .weightOfTimeline = config.weightOfTimeline * prob,
                            .mode = EMode::FindAnswer,
                            .depth = kDepth
                        },
                        space + "  "
                    );
                }
            }
            return result;
        }
    }

    // Нельзя пропускать ход, если можно сделать что-то не помеченное как optional
    if (hasNotOptionalAction) {
        if (config.mode == EMode::FindAnswer) {
            if (bestMove == nullptr) {
                std::cerr << "nullptr 2" << std::endl;
            }
            result = 0;
            for (auto& [prob, world] : (*bestMove)) {
                result += prob * GetWinProbability(
                    world,
                    TConfig{
                        .alpha = 0.,
                        .beta = 1.,
                        .weightOfTimeline = config.weightOfTimeline * prob,
                        .mode = EMode::FindAnswer,
                        .depth = kDepth
                    },
                    space + "  "
                );
            }
        }
        return result;
    }
    battleField.OnTurnEnd();

    //std::cerr << space << "EndTurn" << std::endl;

    updateResult(GetWinProbability(
        battleField,
        TConfig{
            .alpha = config.alpha,
            .beta = config.beta,
            .weightOfTimeline = config.weightOfTimeline,
            .mode = EMode::FindBestMove,
            .depth = config.depth - 1
        },
        space + "  "), nullptr);

    if (config.mode == EMode::FindAnswer) {
        if (bestMove == nullptr) {
            return GetWinProbability(
                battleField,
                TConfig{
                    .alpha = 0.,
                    .beta = 1.,
                    .weightOfTimeline = config.weightOfTimeline,
                    .mode = EMode::FindAnswer,
                    .depth = kDepth
                },
                space + " "
            );
        } else {
            result = 0;
            for (auto& [prob, world] : (*bestMove)) {
                result += prob * GetWinProbability(
                    world,
                    TConfig{
                        .alpha = 0.,
                        .beta = 1.,
                        .weightOfTimeline = config.weightOfTimeline * prob,
                        .mode = EMode::FindAnswer,
                        .depth = kDepth
                    },
                    space + "  "
                );
            }
        }
    }
    return result;
}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TMeleeAttack& action, bool& hasAction)
{
    std::vector<TParallelWorlds> multiverse;
    for (auto i = 0; i < battleField.creatures.size(); ++i) {
        if (battleField.creatures[i].team == battleField.activeCreature->team) {
            continue;
        }
        TBattleField newBattleField(battleField);
        if (action.CanAttack(&(*newBattleField.activeCreature), &newBattleField.creatures[i])) {
            hasAction |= !action.IsOptional();
            multiverse.emplace_back();
            action.Attack(&newBattleField, newBattleField.activeCreature - newBattleField.creatures.begin(), i, &multiverse.back());
        }
    }
    return multiverse;
}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TRangeAttack& action, bool& hasAction)
{
    std::vector<TParallelWorlds> multiverse;
    for (auto i = 0; i < battleField.creatures.size(); ++i) {
        if (battleField.creatures[i].team == battleField.activeCreature->team) {
            continue;
        }
        TBattleField newBattleField(battleField);
        if (action.CanAttack(&(*newBattleField.activeCreature), &newBattleField.creatures[i])) {
            hasAction |= !action.IsOptional();
            multiverse.emplace_back();
            action.Attack(&newBattleField, newBattleField.activeCreature - newBattleField.creatures.begin(), i, &multiverse.back());
        }
    }
    return multiverse;
}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TSpellTargetSelf& action, bool& hasAction)
{
    if (!action.CanCast(&(*battleField.activeCreature))) {
        return {};
    }

    std::vector<TParallelWorlds> multiverse;
    multiverse.emplace_back();
    action.Cast(&battleField, battleField.activeCreature - battleField.creatures.begin(), &multiverse.back());
    return multiverse;
}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TSpellTargetAlly& action, bool& hasAction)
{
    std::vector<TParallelWorlds> multiverse;
    for (auto i = 0; i < battleField.creatures.size(); ++i) {
        if (battleField.creatures[i].team != battleField.activeCreature->team) {
            continue;
        }
        TBattleField newBattleField(battleField);
        if (action.CanCast(&(*newBattleField.activeCreature), &newBattleField.creatures[i])) {
            hasAction |= !action.IsOptional();
            multiverse.emplace_back();
            action.Cast(&newBattleField, newBattleField.activeCreature - newBattleField.creatures.begin(), i, &multiverse.back());
        }
    }
    return multiverse;
}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TSpellTargetEnemy& action, bool& hasAction)
{
    std::vector<TParallelWorlds> multiverse;
    for (auto i = 0; i < battleField.creatures.size(); ++i) {
        if (battleField.creatures[i].team == battleField.activeCreature->team) {
            continue;
        }
        TBattleField newBattleField(battleField);
        if (action.CanCast(&(*newBattleField.activeCreature), &newBattleField.creatures[i])) {
            hasAction |= !action.IsOptional();
            multiverse.emplace_back();
            action.Cast(&newBattleField, newBattleField.activeCreature - newBattleField.creatures.begin(), i, &multiverse.back());
        }
    }
    return multiverse;
}

namespace {

using TAreaCreatureSet = std::pair<std::vector<int>, std::vector<int>>;

}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TSpellAreaEnemy& action, bool& hasAction)
{
    std::vector<TParallelWorlds> multiverse;

    // TODO: make faster
    // Переберем куда используем заклинание. Используется манхетенское расстояние
    std::set<TAreaCreatureSet> wayToCast;

    for (int top_x_area = 0; top_x_area + action.size < height; ++top_x_area) {
        for (int left_y_area = 0; left_y_area + action.size < width; ++left_y_area) {
            TAreaCreatureSet creatureSet;
            for (int i = 0; i < battleField.creatures.size(); ++i) {
                if (battleField.creatures[i].position.x < top_x_area || battleField.creatures[i].position.x >= top_x_area + action.size) {
                    continue;
                }
                if (battleField.creatures[i].position.y < left_y_area || battleField.creatures[i].position.y >= left_y_area + action.size) {
                    continue;
                }
                if (battleField.activeCreature->team == battleField.activeCreature[i].team) {
                    creatureSet.first.emplace_back(i);
                } else {
                    creatureSet.second.emplace_back(i);
                }
            }
            if (creatureSet.second.empty()) {
                continue;
            }
            std::sort(creatureSet.first.begin(), creatureSet.first.end());
            std::sort(creatureSet.second.begin(), creatureSet.second.end());
            wayToCast.insert(creatureSet);
        }
    }

    for (auto& way : wayToCast) {
        std::vector<int> creatures;
        for (int i : way.first) {
            creatures.emplace_back(i);
        }
        for (int i : way.second) {
            creatures.emplace_back(i);
        }

        TBattleField newBattleField(battleField);
        hasAction |= !action.IsOptional();
        multiverse.emplace_back();
        action.Cast(&newBattleField, newBattleField.activeCreature - newBattleField.creatures.begin(), creatures, &multiverse.back());
    }

    return multiverse;
}

std::vector<TParallelWorlds> TEncounter::MakeAction(const TBattleField& battleField, const TBaseAction* actionPtr, bool& hasAction)
{
    if (!actionPtr->CheckResources(battleField.activeCreature->resources)) {
        return {};
    }
    switch (actionPtr->Type()) {
        case EActionType::MeleeAttack: {
            return MakeAction(battleField, *static_cast<const TMeleeAttack*>(actionPtr), hasAction);
        }
        case EActionType::RangeAttack: {
            return MakeAction(battleField, *static_cast<const TRangeAttack*>(actionPtr), hasAction);
        }
        case EActionType::SpellAreaEnemy: {
            return MakeAction(battleField, *static_cast<const TSpellAreaEnemy*>(actionPtr), hasAction);
        }
        case EActionType::SpellTargetAlly: {
            return MakeAction(battleField, *static_cast<const TSpellTargetAlly*>(actionPtr), hasAction);
        }
        case EActionType::SpellTargetEnemy: {
            return MakeAction(battleField, *static_cast<const TSpellTargetEnemy*>(actionPtr), hasAction);
        }
        case EActionType::SpellTargetSelf: {
            return MakeAction(battleField, *static_cast<const TSpellTargetSelf*>(actionPtr), hasAction);
        }
    }
}
