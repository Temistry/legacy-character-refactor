#include "Character.h"

#include "CharacterMath.h"

namespace portfolio
{
Character::Character(CharacterKind kind)
    : id_(-1),
      kind_(kind),
      health_(0),
      energy_(0),
      alive_(false)
{
}

void Character::Initialize(int id, int health)
{
    id_ = id;
    health_ = health;
    energy_ = 0;
    alive_ = health_ > 0;
    events_.clear();
    effects_.Clear();

    EmitEvent("Initialized", id_);
    OnInitialized();
    passives_.Execute(PassiveTiming::OneShot, *this, events_);

    // A tiny generic effect keeps the sample focused on timer ownership,
    // not on real gameplay semantics.
    effects_.AddEffect(2);
}

void Character::Update(const UpdateInput& input)
{
    if (!alive_)
    {
        return;
    }

    EmitEvent("PreUpdate", input.frame);
    OnPreUpdate(input);
    ai_.Dispatch(*this, input, events_);
    effects_.Update(events_);
    passives_.Execute(PassiveTiming::Passive, *this, events_);

    if (input.requestProjectile && shotPattern_.get() != nullptr)
    {
        shotPattern_->Emit(*this, input, events_);
    }

    returnRequest_.TryReturnRequest(*this, input, events_);
    passives_.Execute(PassiveTiming::PostPassive, *this, events_);
    OnPostUpdate(input);
    EmitEvent("Updated", input.frame);
}

void Character::ApplyDamage(int rawDamage)
{
    if (!alive_)
    {
        return;
    }

    const int damage = CharacterMath::ClampDamage(rawDamage);
    health_ -= damage;
    EmitEvent("Damaged", damage);

    if (health_ <= 0)
    {
        passives_.Execute(PassiveTiming::Dying, *this, events_);
        Die();
    }
}

CharacterStateSnapshot Character::Snapshot() const
{
    return CharacterMath::BuildSnapshot(
        id_,
        kind_,
        health_,
        energy_,
        effects_.ActiveCount(),
        alive_);
}

const std::vector<CharacterEvent>& Character::Events() const
{
    return events_;
}

void Character::ClearEvents()
{
    events_.clear();
}

int Character::Id() const
{
    return id_;
}

CharacterKind Character::Kind() const
{
    return kind_;
}

int Character::Health() const
{
    return health_;
}

int Character::Energy() const
{
    return energy_;
}

bool Character::IsAlive() const
{
    return alive_;
}

void Character::AddEnergy(int amount)
{
    energy_ += amount;
}

void Character::SetPassiveRegistry(const PassiveRegistry& registry)
{
    passives_ = registry;
}

void Character::SetShotPattern(std::unique_ptr<ShotPattern> pattern)
{
    shotPattern_ = std::move(pattern);
}

void Character::OnPreUpdate(const UpdateInput&)
{
    AddEnergy(CharacterMath::EnergyGainForKind(kind_));
}

void Character::EmitEvent(const std::string& name, int value)
{
    CharacterEvent event;
    event.name = name;
    event.value = value;
    events_.push_back(event);
}

void Character::Die()
{
    alive_ = false;
    OnDied();
    EmitEvent("Died", id_);
}

PlayerCharacter::PlayerCharacter()
    : Character(CharacterKind::Player)
{
}

void PlayerCharacter::OnPreUpdate(const UpdateInput& input)
{
    Character::OnPreUpdate(input);
}

NpcCharacter::NpcCharacter()
    : Character(CharacterKind::Npc)
{
}

MonsterCharacter::MonsterCharacter()
    : Character(CharacterKind::Monster)
{
}

void MonsterCharacter::OnPreUpdate(const UpdateInput& input)
{
    Character::OnPreUpdate(input);
}

SummonCharacter::SummonCharacter()
    : Character(CharacterKind::Summon)
{
}

ObjectCharacter::ObjectCharacter()
    : Character(CharacterKind::Object)
{
}

EliteMonsterCharacter::EliteMonsterCharacter()
    : Character(CharacterKind::EliteMonster)
{
}

void EliteMonsterCharacter::OnPreUpdate(const UpdateInput& input)
{
    Character::OnPreUpdate(input);
}
}
