#include "LegacyCharacter.h"

#include <algorithm>

namespace portfolio
{
std::array<LegacyCharacter, kLegacyRegistrySize> LegacyRegistry;

LegacyCharacter::LegacyCharacter()
    : id_(-1),
      kind_(CharacterKind::Object),
      health_(0),
      energy_(0),
      effectTicks_(0),
      alive_(false)
{
}

void LegacyCharacter::Initialize(int id, CharacterKind kind, int health)
{
    id_ = id;
    kind_ = kind;
    health_ = health;
    energy_ = 0;
    alive_ = health_ > 0;
    effectTicks_ = 2;
    events_.clear();

    EmitEvent("Initialized", id_);
    TriggerPassive(PassiveTiming::OneShot);
}

void LegacyCharacter::Update(const UpdateInput& input)
{
    if (!alive_)
    {
        return;
    }

    EmitEvent("PreUpdate", input.frame);

    // In the legacy shape, type-specific behavior tends to accumulate here.
    energy_ += EnergyGainForKind();

    UpdateEffects();
    TriggerPassive(PassiveTiming::Passive);

    if (input.requestProjectile)
    {
        FireProjectile(input.nearbyEnemyCount);
    }

    TryReturnRequest(input.requestReturn);
    TriggerPassive(PassiveTiming::PostPassive);
    EmitEvent("Updated", input.frame);
}

void LegacyCharacter::ApplyDamage(int rawDamage)
{
    if (!alive_)
    {
        return;
    }

    const int damage = ClampDamage(rawDamage);
    health_ -= damage;
    EmitEvent("Damaged", damage);

    if (health_ <= 0)
    {
        TriggerPassive(PassiveTiming::Dying);
        Die();
    }
}

CharacterStateSnapshot LegacyCharacter::Snapshot() const
{
    CharacterStateSnapshot snapshot;
    snapshot.id = id_;
    snapshot.kind = kind_;
    snapshot.health = health_;
    snapshot.energy = energy_;
    snapshot.activeEffectCount = effectTicks_ > 0 ? 1 : 0;
    snapshot.alive = alive_;
    return snapshot;
}

const std::vector<CharacterEvent>& LegacyCharacter::Events() const
{
    return events_;
}

void LegacyCharacter::ClearEvents()
{
    events_.clear();
}

void LegacyCharacter::EmitEvent(const std::string& name, int value)
{
    CharacterEvent event;
    event.name = name;
    event.value = value;
    events_.push_back(event);
}

void LegacyCharacter::TriggerPassive(PassiveTiming timing)
{
    switch (timing)
    {
    case PassiveTiming::OneShot:
        energy_ += 1;
        EmitEvent("Passive:OneShot", energy_);
        break;
    case PassiveTiming::Passive:
        energy_ += 1;
        EmitEvent("Passive:Passive", energy_);
        break;
    case PassiveTiming::PostPassive:
        EmitEvent("Passive:PostPassive", energy_);
        break;
    case PassiveTiming::Dying:
        EmitEvent("Passive:Dying", energy_);
        break;
    default:
        break;
    }
}

void LegacyCharacter::UpdateEffects()
{
    if (effectTicks_ <= 0)
    {
        return;
    }

    --effectTicks_;
    if (effectTicks_ == 0)
    {
        EmitEvent("EffectExpired", 0);
    }
}

void LegacyCharacter::FireProjectile(int nearbyEnemyCount)
{
    const int projectileCount = ProjectileCountForKind(nearbyEnemyCount);
    if (projectileCount > 0)
    {
        EmitEvent("ProjectileSpawned", projectileCount);
    }
}

void LegacyCharacter::TryReturnRequest(bool requested)
{
    if (requested)
    {
        EmitEvent("ReturnRequestChecked", 1);
    }
}

int LegacyCharacter::ClampDamage(int rawDamage) const
{
    return std::max(0, rawDamage);
}

int LegacyCharacter::EnergyGainForKind() const
{
    switch (kind_)
    {
    case CharacterKind::Player:
        return 1;
    case CharacterKind::Monster:
        return 2;
    case CharacterKind::EliteMonster:
        return 3;
    default:
        return 0;
    }
}

int LegacyCharacter::ProjectileCountForKind(int nearbyEnemyCount) const
{
    switch (kind_)
    {
    case CharacterKind::Monster:
        return std::max(1, nearbyEnemyCount);
    case CharacterKind::EliteMonster:
        return std::max(1, nearbyEnemyCount + 1);
    case CharacterKind::Summon:
        return nearbyEnemyCount > 0 ? 1 : 0;
    default:
        return 1;
    }
}

void LegacyCharacter::Die()
{
    alive_ = false;
    EmitEvent("Died", id_);
}

LegacyCharacter& LegacyAt(std::size_t index)
{
    return LegacyRegistry[index];
}
}

