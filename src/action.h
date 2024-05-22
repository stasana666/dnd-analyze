#pragma once

#include "resources.h"
#include "damage.h"
#include "status.h"

#include "parser/parser.h"

#include "characteristics.h"

#include <vector>

#include <rapidjson/document.h>

class TCreature;
class TBattleField;

enum EActionType {
    MeleeAttack,
    RangeAttack,
    SpellTargetSelf,
    SpellTargetAlly,
    SpellTargetEnemy,
    SpellAreaEnemy,
};

class TBaseAction {
public:
    TBaseAction(TParser& parser, const rapidjson::Value& json);

    bool CheckResources(const TResources& resources) const;
    void ChangeResources(TResources& resources) const;

    bool IsOptional() const {return isOptionalAction; }
    void DivWorldByDeath(const TBattleField* battleField, std::vector<int> targetId,
        std::vector<std::pair<double, TBattleField>>* battles, double probWorlds) const;

    std::string Name() const { return name; }

    virtual EActionType Type() const = 0;

protected:
    std::string name;
    bool isOptionalAction;
    std::vector<TResource> consumeOnSuccess;
    std::vector<TResource> consumeResources;
    std::vector<TResource> recoverResources;
};

class TBaseAttack : public TBaseAction{
public:
    TBaseAttack(TParser& parser, const rapidjson::Value& json);

protected:
    TDamage damage;
    TDamage critDamage;
    double critProbability;
    int attackBonus;
};

class TMeleeAttack final : public TBaseAttack {
public:
    TMeleeAttack(TParser& parser, const rapidjson::Value& json);

    bool CanAttack(const TCreature* parent, const TCreature* target) const;
    void Attack(const TBattleField* battleField, int creatureId, int targetId, std::vector<std::pair<double, TBattleField>>* battles) const;

    EActionType Type() const override { return EActionType::MeleeAttack; }
};

class TRangeAttack final : public TBaseAttack {
public:
    TRangeAttack(TParser& parser, const rapidjson::Value& json);

    bool CanAttack(const TCreature* parent, const TCreature* target) const;
    void Attack(const TBattleField* battleField, int creatureId, int targetId, std::vector<std::pair<double, TBattleField>>* battles) const;

    EActionType Type() const override { return EActionType::RangeAttack; }
private:
    int distance;
};

class TBaseSpell : public TBaseAction {
public:
    TBaseSpell(TParser& parser, const rapidjson::Value& json);

protected:
    double ProbFailSaveThrow(const TCreature* target) const;

    std::optional<TDamage> damage;
    std::optional<TDistribution> heal;
    std::vector<EStatus> addNewStatus;
    std::optional<TStats::EStat> savethrow;
    int difficulty;
};

class TSpellTargetSelf : public TBaseSpell {
public:
    TSpellTargetSelf(TParser& parser, const rapidjson::Value& json);

    bool CanCast(const TCreature* creature) const;
    void Cast(const TBattleField* battleField, int creatureId, std::vector<std::pair<double, TBattleField>>* battles) const;

    EActionType Type() const override { return EActionType::SpellTargetSelf; }
};

class TSpellTargetAlly : public TBaseSpell {
public:
    TSpellTargetAlly(TParser& parser, const rapidjson::Value& json);

    bool CanCast(const TCreature* creature, const TCreature* target) const;
    void Cast(const TBattleField* battleField, int creatureId, int targetId, std::vector<std::pair<double, TBattleField>>* battles) const;

    EActionType Type() const override { return EActionType::SpellTargetAlly; }
};

class TSpellTargetEnemy : public TBaseSpell {
public:
    TSpellTargetEnemy(TParser& parser, const rapidjson::Value& json);

    bool CanCast(const TCreature* creature, const TCreature* target) const;
    void Cast(const TBattleField* battleField, int creatureId, int targetId, std::vector<std::pair<double, TBattleField>>* battles) const;

    EActionType Type() const override { return EActionType::SpellTargetEnemy; }
};

class TSpellAreaEnemy : public TBaseSpell {
public:
    TSpellAreaEnemy(TParser& parser, const rapidjson::Value& json);

    void Cast(const TBattleField* battleField, int creatureId, const std::vector<int>& targets, std::vector<std::pair<double, TBattleField>>* battles) const;

    EActionType Type() const override { return EActionType::SpellAreaEnemy; }

    int size;
};
