#pragma once

#include "AI/AiDispatcher.h"
#include "CharacterTypes.h"
#include "Effects/TimedEffectList.h"
#include "Passive/PassiveRegistry.h"
#include "Projectile/ShotPattern.h"
#include "Return/ReturnRequestModule.h"

#include <memory>
#include <vector>

namespace portfolio
{
// Legacy shape:
//   LegacyCharacter::Update() owned AI, timers, passive timing, projectile
//   dispatch, return-request handling, and event ordering in one method.
//
// Refactored shape:
//   Character keeps the lifecycle order stable and delegates narrow work to
//   collaborators. Derived classes only override hooks for type-specific parts.
//
// Visual flow:
//   Initialize
//     -> Update: PreUpdate -> hook -> AI -> effects -> passive -> shot
//               -> return-request -> post-passive -> hook -> Updated
//     -> ApplyDamage -> Dying passive -> Die
class Character
{
public:
    explicit Character(CharacterKind kind);
    virtual ~Character() {}

    void Initialize(int id, int health);
    void Update(const UpdateInput& input);
    void ApplyDamage(int rawDamage);

    CharacterStateSnapshot Snapshot() const;
    const std::vector<CharacterEvent>& Events() const;
    void ClearEvents();

    int Id() const;
    CharacterKind Kind() const;
    int Health() const;
    int Energy() const;
    bool IsAlive() const;

    void AddEnergy(int amount);
    void SetPassiveRegistry(const PassiveRegistry& registry);
    void SetShotPattern(std::unique_ptr<ShotPattern> pattern);

protected:
    virtual void OnInitialized() {}
    virtual void OnPreUpdate(const UpdateInput& input);
    virtual void OnPostUpdate(const UpdateInput& input) {}
    virtual void OnDied() {}

    void EmitEvent(const std::string& name, int value);

private:
    void Die();

    int id_;
    CharacterKind kind_;
    int health_;
    int energy_;
    bool alive_;
    std::vector<CharacterEvent> events_;
    TimedEffectList effects_;
    PassiveRegistry passives_;
    std::unique_ptr<ShotPattern> shotPattern_;
    ReturnRequestModule returnRequest_;
    AiDispatcher ai_;
};

class PlayerCharacter : public Character
{
public:
    PlayerCharacter();

protected:
    void OnPreUpdate(const UpdateInput& input) override;
};

class NpcCharacter : public Character
{
public:
    NpcCharacter();
};

class MonsterCharacter : public Character
{
public:
    MonsterCharacter();

protected:
    void OnPreUpdate(const UpdateInput& input) override;
};

class SummonCharacter : public Character
{
public:
    SummonCharacter();
};

class ObjectCharacter : public Character
{
public:
    ObjectCharacter();
};

class EliteMonsterCharacter : public Character
{
public:
    EliteMonsterCharacter();

protected:
    void OnPreUpdate(const UpdateInput& input) override;
};
}
