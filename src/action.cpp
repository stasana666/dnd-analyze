#include "action.h"

#include "resources.h"
#include "creature.h"
#include "battlefield.h"
#include "status.h"

#include <iostream>
#include <algorithm>

namespace {

constexpr double kDeathEps = 0.01;

void Normalize(TParallelWorlds& worlds)
{
    double sum = 0.;
    for (auto& [prob, _] : worlds) {
        sum += prob;
    }
    for (auto& [prob, _] : worlds) {
        prob /= sum;
    }
}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TBaseAction::TBaseAction(TParser& parser, const rapidjson::Value& json)
    : name(json["name"].GetString())
{
    auto readResources = [](std::vector<TResource>& resources, const rapidjson::Value& json) {
        for (auto& resource : json.GetArray()) {
            resources.emplace_back(resource);
        }
    };
    if (json.HasMember("consume_resources")) {
        readResources(consumeResources, json["consume_resources"]);
    }
    if (json.HasMember("consume_on_success")) {
        readResources(consumeOnSuccess, json["consume_on_success"]);
    }
    if (json.HasMember("recover_resources")) {
        readResources(recoverResources, json["recover_resources"]);
    }
}

bool TBaseAction::CheckResources(const TResources& resources) const
{
    for (auto res : consumeResources) {
        if (!resources.Check(res)) {
            return false;
        }
    }
    for (auto res : consumeOnSuccess) {
        if (!resources.Check(res)) {
            return false;
        }
    }
    return true;
}

void TBaseAction::ChangeResources(TResources& resources) const
{
    for (auto res : consumeResources) {
        resources.Consume(res);
    }
    for (auto res : recoverResources) {
        resources.AddTmpResource(res);
    }
}

void TBaseAction::DivWorldByDeath(const TBattleField* battleField, std::vector<int> targetIds,
    TParallelWorlds* battles, double probWorlds) const
{
    std::sort(targetIds.begin(), targetIds.end());
    std::reverse(targetIds.begin(), targetIds.end());

    // Переберем все возможные исходы. биты означают 1 - жив, 0 - мертв
    std::vector<double> probsDead;
    for (size_t i : targetIds) {
        probsDead.emplace_back(battleField->creatures[i].hitpoints < 1);
    }
    TParallelWorlds worlds;
    for (int mask = 0; mask < (1 << targetIds.size()); ++mask) {
        double worldProb = 1;
        for (int i = 0; i < targetIds.size(); ++i) {
            if (mask & (1 << i)) {
                worldProb *= 1 - probsDead[i];
            } else {
                worldProb *= probsDead[i];
            }
        }
        if (worldProb < kDeathEps) {
            continue;
        }
        worlds.emplace_back(worldProb, *battleField);
        for (int i = 0; i < targetIds.size(); ++i) {
            if (mask & (1 << i)) {
                worlds.back().second.creatures[targetIds[i]].hitpoints.DelBelowZero();
            } else {
                if (worlds.back().second.ActiveCreatureId() > targetIds[i]) {
                    --worlds.back().second.activeCreature;
                }
                worlds.back().second.creatures.erase(worlds.back().second.creatures.begin() + targetIds[i]);
            }
        }
    }
    Normalize(worlds);
    for (auto& [prob, _] : worlds) {
        prob *= probWorlds;
    }
    for (auto& world : worlds) {
        battles->emplace_back(std::move(world));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TBaseAttack::TBaseAttack(TParser& parser, const rapidjson::Value& json)
    : TBaseAction(parser, json)
    , damage(json["damage"], parser)
    , critDamage(json["crit_damage"], parser)
    , attackBonus(parser.Parse(std::string(json["attack"].GetString())))
    , critProbability(json.HasMember("crit_range") ? (21. - json["crit_range"].GetInt()) / 20. : (1. / 20.))
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TMeleeAttack::TMeleeAttack(TParser& parser, const rapidjson::Value& json)
    : TBaseAttack(parser, json)
{
}

bool TMeleeAttack::CanAttack(const TCreature* creature, const TCreature* target) const
{
    // В D&D много вариантов, как можно считать расстояния. Будем использовать самый простой - манхетенское.
    if (std::abs(creature->position.x - target->position.x) > 1 + target->speed) {
        return false;
    }
    if (std::abs(creature->position.y - target->position.y) > 1 + target->speed) {
        return false;
    }
    return CheckResources(creature->resources);
}

void TMeleeAttack::Attack(const TBattleField* battleField, int creatureId, int targetId, TParallelWorlds* battles) const
{
    TBattleField battle = *battleField;
    TCreature* creature = &battle.creatures[creatureId];
    TCreature* target = &battle.creatures[targetId];

    ChangeResources(creature->resources);
    double failProbability = std::min(1. - critProbability, std::max(1. / 20., (target->parent->armourClass - attackBonus - 1) / 20.));
    double successProbability = std::max(0., 1. - critProbability - failProbability);
    
    if (consumeOnSuccess.empty()) {
        // Тут не нужно разделять успех и провал атаки, просто наносим урон
        std::vector<std::pair<double, TDistribution>> damages;
        damages.emplace_back(critProbability, critDamage.GetDistributionFor(target));
        if (successProbability > 0) {
            damages.emplace_back(successProbability, damage.GetDistributionFor(target));
        }
        damages.emplace_back(failProbability, TDistribution(0));

        target->hitpoints -= TDistribution(damages);

        DivWorldByDeath(&battle, {targetId}, battles, 1.);
    } else {
        // Добавим мир, в котором ничего не произошло
        battles->emplace_back(failProbability, battle);

        // Добавим миры, где атака попала и ресурс потратился
        std::vector<std::pair<double, TDistribution>> damages;
        damages.emplace_back(critProbability / (1 - failProbability), critDamage.GetDistributionFor(target));
        if (successProbability > 0) {
            damages.emplace_back(successProbability / (1 - failProbability), damage.GetDistributionFor(target));
        }
        target->hitpoints -= TDistribution(damages);
        for (auto res : consumeOnSuccess) {
            creature->resources.Consume(res);
        }
        DivWorldByDeath(&battle, {targetId}, battles, 1 - failProbability);
    }
    Normalize(*battles);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TRangeAttack::TRangeAttack(TParser& parser, const rapidjson::Value& json)
    : TBaseAttack(parser, json)
{
}

bool TRangeAttack::CanAttack(const TCreature* creature, const TCreature* target) const
{
    return CheckResources(creature->resources);
}

void TRangeAttack::Attack(const TBattleField* battleField, int creatureId, int targetId, TParallelWorlds* battles) const
{
    TBattleField battle = *battleField;
    TCreature* creature = &battle.creatures[creatureId];
    TCreature* target = &battle.creatures[targetId];

    ChangeResources(creature->resources);
    double failProbability = std::min(1. - critProbability, std::max(1. / 20., (target->parent->armourClass - attackBonus - 1) / 20.));
    double successProbability = std::max(0., 1. - critProbability - failProbability);

    if (consumeOnSuccess.empty()) {
        // Тут не нужно разделять успех и провал атаки, просто наносим урон
        std::vector<std::pair<double, TDistribution>> damages;
        damages.emplace_back(critProbability, critDamage.GetDistributionFor(target));
        if (successProbability > 0) {
            damages.emplace_back(successProbability, damage.GetDistributionFor(target));
        }
        damages.emplace_back(failProbability, TDistribution(0));

        target->hitpoints -= TDistribution(damages);

        DivWorldByDeath(&battle, {targetId}, battles, 1.);
    } else {
        // Добавим мир, в котором ничего не произошло
        battles->emplace_back(failProbability, battle);

        // Добавим миры, где атака попала и ресурс потратился
        std::vector<std::pair<double, TDistribution>> damages;
        damages.emplace_back(critProbability / (1 - failProbability), critDamage.GetDistributionFor(target));
        if (successProbability > 0) {
            damages.emplace_back(successProbability / (1 - failProbability), damage.GetDistributionFor(target));
        }
        target->hitpoints -= TDistribution(damages);
        for (auto res : consumeOnSuccess) {
            target->resources.Consume(res);
        }
        DivWorldByDeath(&battle, {targetId}, battles, 1 - failProbability);
    }
    Normalize(*battles);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TBaseSpell::TBaseSpell(TParser& parser, const rapidjson::Value& json)
    : TBaseAction(parser, json)
{
    if (json.HasMember("damage")) {
        damage = TDamage(json["damage"], parser);
    }
    if (json.HasMember("heal")) {
        heal.emplace(parser.ParseDistribution(json["heal"].GetString()));
    }
    if (json.HasMember("savethrow")) {
        savethrow.emplace(TStats::GetStatByString(json["savethrow"].GetString()));
        difficulty = parser.Parse(json["difficulty"].GetString());
    }
}

double TBaseSpell::ProbFailSaveThrow(const TCreature* target) const {
    if (!savethrow.has_value()) {
        return 1.;
    }
    return std::min(1., std::max(0., (1. / 20.) * (difficulty - target->parent->savethrows[savethrow.value()] - 1)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSpellTargetSelf::TSpellTargetSelf(TParser& parser, const rapidjson::Value& json)
    : TBaseSpell(parser, json)
{
    if (json.HasMember("add_status")) {
        for (auto& statusName : json["add_status"].GetArray()) {
            addNewStatus.emplace_back(GetStatusByString(statusName.GetString()));
        }
    }
}

bool TSpellTargetSelf::CanCast(const TCreature* creature) const
{
    return CheckResources(creature->resources);
}

void TSpellTargetSelf::Cast(const TBattleField* battleField, int creatureId, std::vector<std::pair<double, TBattleField>>* battles) const
{
    battles->emplace_back(1., TBattleField(*battleField));
    TCreature& creature = battles->back().second.creatures[creatureId];
    
    ChangeResources(creature.resources);
    for (auto& status : addNewStatus) {
        creature.status.emplace_back(status);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSpellTargetAlly::TSpellTargetAlly(TParser& parser, const rapidjson::Value& json)
    : TBaseSpell(parser, json)
{
}

bool TSpellTargetAlly::CanCast(const TCreature* creature, const TCreature* target) const
{
    return CheckResources(creature->resources);
}

void TSpellTargetAlly::Cast(const TBattleField* battleField, int creatureId,
    int targetId, TParallelWorlds* battles) const
{
    battles->emplace_back(1., TBattleField(*battleField));
    TCreature& creature = battles->back().second.creatures[creatureId];

    ChangeResources(creature.resources);
    if (heal.has_value()) {
        creature.hitpoints += heal.value();
        creature.hitpoints.MakeMaxValue(creature.parent->maxHp);
    }

    for (auto& status : addNewStatus) {
        creature.status.emplace_back(status);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSpellTargetEnemy::TSpellTargetEnemy(TParser& parser, const rapidjson::Value& json)
    : TBaseSpell(parser, json)
{
}

bool TSpellTargetEnemy::CanCast(const TCreature* creature, const TCreature* target) const
{
    return CheckResources(creature->resources);
}

void TSpellTargetEnemy::Cast(const TBattleField* battleField, int creatureId,
    int targetId, TParallelWorlds* battles) const
{
    double probFailSaveThrow = ProbFailSaveThrow(&battleField->creatures[targetId]);

    if (probFailSaveThrow < 1 - 1e-2 && !addNewStatus.empty()) {
        battles->emplace_back(probFailSaveThrow, TBattleField(*battleField));
        ChangeResources(battles->back().second.creatures[creatureId].resources);
        battles->emplace_back(1. - probFailSaveThrow, TBattleField(battles->back().second));

        const TCreature& target = battleField->creatures[targetId];

        if (damage.has_value()) {
            (*battles)[0].second.creatures[targetId].hitpoints -= damage.value().GetDistributionFor(&target);
            (*battles)[1].second.creatures[targetId].hitpoints -= damage.value().GetDistributionFor(&target) / 2;
        }

        for (auto& status : addNewStatus) {
            (*battles)[0].second.creatures[targetId].status.emplace_back(status);
        }
    } else {
        battles->emplace_back(1., TBattleField(*battleField));
        TCreature& creature = battles->back().second.creatures[creatureId];
        ChangeResources(creature.resources);
        TCreature& target = battles->back().second.creatures[targetId];

        if (damage.has_value()) {
            TDistribution realDamage = damage.value().GetDistributionFor(&target);
            TDistribution afterSave(std::vector<std::pair<double, TDistribution>>({{1. - probFailSaveThrow, realDamage / 2}, {probFailSaveThrow, realDamage}}));
            creature.hitpoints -= afterSave;
        }

        for (auto& status : addNewStatus) {
            target.status.emplace_back(status);
        }
    }

    for (int i = static_cast<int>(battles->size()) - 1; i >= 0; --i) {
        TBattleField& battle = (*battles)[i].second;
        double probDeath = battle.creatures[targetId].hitpoints < 0;
        double probLife = 1. - probDeath;
        if (probDeath > kDeathEps) {
            TBattleField copy(battle);
            copy.creatures.erase(copy.creatures.begin() + targetId);
            battles->emplace_back(probDeath, copy);
            if (probLife < kDeathEps) {
                battles->erase(battles->begin() + i);
            } else {
                battle.creatures[targetId].hitpoints.DelBelowZero();
            }
        }
    }

    Normalize(*battles);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Область это квадрат size на size клеток
TSpellAreaEnemy::TSpellAreaEnemy(TParser& parser, const rapidjson::Value& json)
    : TBaseSpell(parser, json)
    , size(json["size"].GetInt())
{
}

void TSpellAreaEnemy::Cast(const TBattleField* battleField, int creatureId,
    const std::vector<int>& targets, TParallelWorlds* battles) const
{
    battles->emplace_back(1., *battleField);
    ChangeResources(battles->back().second.creatures[creatureId].resources);

    for (int i : targets) {
        TCreature& target = battles->back().second.creatures[i];
        TDistribution targetDamage = damage.value().GetDistributionFor(&target);
        double probFailSave = ProbFailSaveThrow(&target);
        target.hitpoints -= TDistribution(std::vector<std::pair<double, TDistribution>>{{probFailSave, targetDamage / 2}, {1. - probFailSave, targetDamage}});
    }
}
