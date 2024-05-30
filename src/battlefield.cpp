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
    , turnStart(false)
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
    , turnStart(battleField.turnStart)
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
    turnStart = true;
}

void TBattleField::OnTurnStart()
{
    activeCreature->resources.OnTurnStart();
    activeCreature->speed = 6;
    turnStart = false;
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

TEncounter::TEncounter(int width, int height)
    : width(width)
    , height(height)
{
}

void TEncounter::AddCreature(std::shared_ptr<const TStatblock> statblock, int x, int y, int team)
{
    creatures.emplace_back(statblock);
    creatures.back().position = TPosition{x, y};
    creatures.back().team = team;
}

TParallelWorlds TEncounter::OnTurnStart(TBattleField battleField) const
{
    battleField.OnTurnStart();
    TParallelWorlds result;
    auto resources = battleField.activeCreature->resources.NeedRecover56();
    if (battleField.round == 1) {
        for (auto res : resources) {
            battleField.activeCreature->resources.Add(TResource(res, 1));
        }
        result.emplace_back(1., battleField);
        return result;
    }
    for (int mask = 0; mask < (1 << resources.size()); ++mask) {
        double prob = 1.;
        TBattleField world(battleField);
        for (int i = 0; i < resources.size(); ++i) {
            if (mask & (1 << i)) {
                prob *= (1. / 3.);
                world.activeCreature->resources.Add(TResource(resources[i], 1));
            } else {
                prob *= (2. / 3.);
            }
        }
        result.emplace_back(prob, world);
    }
    if (result.empty()) {
        result.emplace_back(1, battleField);
    }
    return result;
}

TResult TEncounter::GetWinProbability()
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

TResult TEncounter::GetWinProbability(TBattleField battleField, TConfig config, std::string space)
{
    if (battleField.turnStart) {
        TParallelWorlds worlds = OnTurnStart(battleField);
        TResult fullResult(0, -1);
        for (auto& [prob, world] : worlds) {
            fullResult += GetWinProbability(
                world,
                config,
                space + "  "
            ) * prob;
        }
        return fullResult;
    }
    if (config.weightOfTimeline < 1e-3) {
        return TResult(battleField.GetEstimate(), -1);
    }

    const int CurrentTeam = battleField.activeCreature->team;
    std::array<int, 2> teamsSize{0, 0};
    for (auto& creature : battleField.creatures) {
        ++teamsSize[creature.team];
    }
    if (teamsSize[0] == 0) {
        return TResult(0, 0);
    }
    if (teamsSize[1] == 0) {
        return TResult(1, teamsSize[0]);
    }

    if (config.mode == EMode::FindBestMove && config.depth == 0) {
        return TResult(battleField.GetEstimate(), -1);
    }

    TResult result(CurrentTeam, -1);
    TParallelWorlds* bestMove = nullptr;
    auto updateResult = [&](TResult localRes, TParallelWorlds* move) {
        if (CurrentTeam == 0) {
            if (localRes > result) {
                bestMove = move;
            }
            config.alpha = std::max(config.alpha, localRes.probWin);
            result = std::max(result, localRes);
        } else if (CurrentTeam == 1) {
            if (localRes < result) {
                bestMove = move;
            }
            config.beta = std::min(config.beta, localRes.probWin);
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
        TResult fullResult(0, -1);
        for (auto& [prob, world] : worlds) {
            fullResult += GetWinProbability(
                world,
                TConfig{
                    .alpha = config.alpha,
                    .beta = config.beta,
                    .weightOfTimeline = config.weightOfTimeline * prob,
                    .mode = EMode::FindBestMove,
                    .depth = config.depth - 1
                },
                space + "  "
            ) * prob;
        }
        if (updateResult(fullResult, &worlds)) {
            // Нашли ожидаемый лучший ход - сделаем его и найдем ответ для новой позиции
            if (config.mode == EMode::FindAnswer) {
                //std::cerr << space << "FindAnswer" << std::endl;
                result = TResult(0, -1);
                for (auto& [prob, world] : (*bestMove)) {
                    result += GetWinProbability(
                        world,
                        TConfig{
                            .alpha = 0.,
                            .beta = 1.,
                            .weightOfTimeline = config.weightOfTimeline * prob,
                            .mode = EMode::FindAnswer,
                            .depth = kDepth
                        },
                        space + "  "
                    ) * prob;
                }
            }
            return result;
        }
    }

    // Нельзя пропускать ход, если можно сделать что-то не помеченное как optional
    if (hasNotOptionalAction) {
        if (config.mode == EMode::FindAnswer) {
            //std::cerr << space << "FindAnswer" << std::endl;
            result = TResult(0, -1);
            for (auto& [prob, world] : (*bestMove)) {
                result += GetWinProbability(
                    world,
                    TConfig{
                        .alpha = 0.,
                        .beta = 1.,
                        .weightOfTimeline = config.weightOfTimeline * prob,
                        .mode = EMode::FindAnswer,
                        .depth = kDepth
                    },
                    space + "  "
                ) * prob;
            }
        }
        return result;
    }
    battleField.OnTurnEnd();

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
        //std::cerr << space << "FindAnswer" << std::endl;
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
            result = TResult(0, -1);
            for (auto& [prob, world] : (*bestMove)) {
                result += GetWinProbability(
                    world,
                    TConfig{
                        .alpha = 0.,
                        .beta = 1.,
                        .weightOfTimeline = config.weightOfTimeline * prob,
                        .mode = EMode::FindAnswer,
                        .depth = kDepth
                    },
                    space + "  "
                ) * prob;
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
                if (battleField.activeCreature->team == battleField.creatures[i].team) {
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

    std::set<TAreaCreatureSet> filteredWayToCast;
    auto isSubSeq = [&](const std::vector<int>& a, const std::vector<int>& b) {
        int ptr = 0;
        for (auto i : a) {
            if (ptr == b.size()) {
                return false;
            }
            if (b[ptr] == i) {
                ++ptr;
                continue;
            }
            ++ptr;
            if (ptr == b.size()) {
                return false;
            }
        }
        return true;
    };
    for (const TAreaCreatureSet& way : wayToCast) {
        bool flag = true;
        for (const TAreaCreatureSet& otherWay : wayToCast) {
            if (way == otherWay) {
                continue;
            }
            if (isSubSeq(otherWay.first, way.first) && isSubSeq(way.second, otherWay.second)) {
                flag = false;
                break;
            }
        }
        if (flag) {
            filteredWayToCast.insert(way);
        }
    }

    wayToCast.swap(filteredWayToCast);

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
