#include "LegacyCharacter.h"

#include <algorithm>

namespace portfolio
{
std::array<LegacyCharacter, kLegacyRegistrySize> LegacyRegistry;
LegacyCharacter* gCharacters[kMaxLegacyCharacterCount] = {};

LegacyCharacter::LegacyCharacter()
    : id_(-1),
      kind_(CharacterKind::Object),
      legacyType_(LegacyCharacterType::Object),
      health_(0),
      energy_(0),
      x_(0),
      y_(0),
      velocityX_(0),
      velocityY_(0),
      aiState_(0),
      targetIndex_(-1),
      attackDelay_(0),
      hitStopTimer_(0),
      effectTicks_(0),
      poisonTimer_(0),
      stunTimer_(0),
      burnTimer_(0),
      shieldTimer_(0),
      slowTimer_(0),
      skillCooldownA_(0),
      skillCooldownB_(0),
      skillCooldownC_(0),
      skillCooldownD_(0),
      passiveCounterA_(0),
      passiveCounterB_(0),
      passiveCounterC_(0),
      summonLifeTime_(0),
      ownerIndex_(-1),
      hostIndex_(-1),
      alive_(false),
      moving_(false),
      attacking_(false),
      passiveFlagA_(false),
      passiveFlagB_(false),
      passiveFlagC_(false),
      temporaryInvulnerable_(false),
      summon_(nullptr)
{
}

LegacyCharacter::~LegacyCharacter()
{
    CleanupSummon();
    DeleteOldProjectiles();
}

void LegacyCharacter::Initialize(int id, CharacterKind kind, int health)
{
    id_ = id;
    kind_ = kind;
    legacyType_ = ToLegacyType(kind);
    health_ = health;
    energy_ = 0;
    alive_ = health_ > 0;
    effectTicks_ = 2;
    events_.clear();
    ResetLegacyState();

    // Legacy compatibility: old call sites expect this global array.
    if (id_ >= 0 && id_ < static_cast<int>(kMaxLegacyCharacterCount))
    {
        gCharacters[id_] = this;
    }

    EmitEvent("Initialized", id_);
    TriggerPassive(PassiveTiming::OneShot);
}

void LegacyCharacter::Update(const UpdateInput& input)
{
    if (!alive_)
    {
        return;
    }

    // Do not change order. Called from many places.
    EmitEvent("PreUpdate", input.frame);

    if (hitStopTimer_ > 0)
    {
        --hitStopTimer_;
    }

    if (stunTimer_ <= 0)
    {
        ProcessAI(input);
        Move(velocityX_, velocityY_);
    }

    // Temporary fix: type-specific energy rules stayed in Update.
    energy_ += EnergyGainForKind();

    UpdateSkillTimers();
    UpdateStatusTimers();
    UpdateLegacyEffectTriggers();
    UpdateEffects();
    ApplyPassiveEffects(LegacyPassiveMoment::PerFrame, input.frame);
    TriggerPassive(PassiveTiming::Passive);

    if (input.requestProjectile)
    {
        FireProjectile(input.nearbyEnemyCount);
    }

    TryReturnRequest(input.requestReturn);

    if (summon_ != nullptr && summon_->active)
    {
        --summon_->lifeTime;
        if (summon_->lifeTime <= 0)
        {
            CleanupSummon();
        }
    }

    DeleteOldProjectiles();
    TriggerPassive(PassiveTiming::PostPassive);
    EmitEvent("Updated", input.frame);
}

void LegacyCharacter::ApplyDamage(int rawDamage)
{
    if (!alive_)
    {
        return;
    }

    ApplyPassiveEffects(LegacyPassiveMoment::OnDamaged, rawDamage);

    int damage = ClampDamage(rawDamage);
    if (shieldTimer_ > 0) { damage = std::max(0, damage - 1); }
    if (temporaryInvulnerable_)
    {
        damage = 0;
        temporaryInvulnerable_ = false;
    }

    health_ -= damage;
    EmitEvent("Damaged", damage);

    if (health_ <= 0)
    {
        ApplyPassiveEffects(LegacyPassiveMoment::BeforeDeath, damage);
        TriggerPassive(PassiveTiming::Dying);
        Die();
    }
}

void LegacyCharacter::Move(int deltaX, int deltaY)
{
    if (!alive_ || stunTimer_ > 0 || legacyType_ == LegacyCharacterType::Object)
    {
        moving_ = false;
        return;
    }

    if (slowTimer_ > 0)
    {
        deltaX /= 2;
        deltaY /= 2;
    }

    x_ += deltaX;
    y_ += deltaY;
    moving_ = deltaX != 0 || deltaY != 0;
}

void LegacyCharacter::Attack(int targetIndex)
{
    if (!alive_ || attackDelay_ > 0)
    {
        return;
    }

    ApplyPassiveEffects(LegacyPassiveMoment::BeforeAttack, targetIndex);
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    if (target == nullptr)
    {
        targetIndex_ = -1;
        return;
    }

    targetIndex_ = targetIndex;
    attacking_ = true;

    if (legacyType_ == LegacyCharacterType::Monster)
    {
        attackDelay_ = 2;
        CreateStraightProjectile(targetIndex);
    }
    else if (legacyType_ == LegacyCharacterType::BossMonster)
    {
        // Special case: copy-pasted from Monster and adjusted later.
        attackDelay_ = 3;
        CreateMultiProjectile(targetIndex);
        if (health_ < 5) { CreateDirectionalProjectile(targetIndex, 1, 0); }
    }
    else if (legacyType_ == LegacyCharacterType::Summon)
    {
        attackDelay_ = 2;
        CreateHomingProjectile(targetIndex);
    }
    else if (legacyType_ != LegacyCharacterType::Object)
    {
        attackDelay_ = 1;
        CreateStraightProjectile(targetIndex);
    }

    ApplyPassiveEffects(LegacyPassiveMoment::AfterAttack, targetIndex);
}

void LegacyCharacter::CastSkill(int skillId, int targetIndex)
{
    // Skill ids are toy values. The point is the accumulated state in this class.
    if (!alive_) { return; }

    if (skillId == 1 && skillCooldownA_ <= 0)
    {
        skillCooldownA_ = 3;
        passiveFlagA_ = true;
        CreateProjectile(targetIndex, skillId);
    }
    else if (skillId == 2 && skillCooldownB_ <= 0)
    {
        skillCooldownB_ = legacyType_ == LegacyCharacterType::BossMonster ? 4 : 3;
        passiveCounterB_ += 1;
        CreateMultiProjectile(targetIndex);
    }
    else if (skillId == 3 && skillCooldownC_ <= 0)
    {
        skillCooldownC_ = 5;
        stunTimer_ = std::max(stunTimer_, 1);
        CreateDirectionalProjectile(targetIndex, 0, 1);
    }
    else if (skillId == 4 && skillCooldownD_ <= 0)
    {
        skillCooldownD_ = 6;
        SummonUnit(targetIndex);
    }
}

void LegacyCharacter::SummonUnit(int targetIndex)
{
    if (LegacyRawAt(targetIndex) == nullptr) { return; }
    if (summon_ != nullptr) { CleanupSummon(); }

    // Allocation policy is scattered across gameplay functions.
    summon_ = new LegacySummonData();
    summon_->ownerId = id_;
    summon_->hostId = targetIndex;
    summon_->active = true;

    if (legacyType_ == LegacyCharacterType::Monster)
    {
        summon_->lifeTime = 2;
    }
    else if (legacyType_ == LegacyCharacterType::BossMonster)
    {
        summon_->lifeTime = 3;
        passiveFlagC_ = true;
    }
    else if (legacyType_ == LegacyCharacterType::Summon)
    {
        summon_->lifeTime = 1;
        ownerIndex_ = targetIndex;
    }
    else
    {
        summon_->lifeTime = 2;
        hostIndex_ = targetIndex;
    }

    summonLifeTime_ = summon_->lifeTime;
}

void LegacyCharacter::RecallUnit()
{
    if (summon_ == nullptr) { return; }

    if (legacyType_ == LegacyCharacterType::Summon && ownerIndex_ >= 0)
    {
        summon_->hostId = ownerIndex_;
    }
    else if (legacyType_ == LegacyCharacterType::Object && hostIndex_ >= 0)
    {
        summon_->hostId = hostIndex_;
    }
    else
    {
        summon_->hostId = id_;
    }
}

void LegacyCharacter::CleanupSummon()
{
    delete summon_;
    summon_ = nullptr;
    summonLifeTime_ = 0;
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

void LegacyCharacter::ResetLegacyState()
{
    x_ = 0; y_ = 0; velocityX_ = 0; velocityY_ = 0;
    aiState_ = 0; targetIndex_ = -1; attackDelay_ = 0; hitStopTimer_ = 0;
    poisonTimer_ = 0; stunTimer_ = 0; burnTimer_ = 0; shieldTimer_ = 0; slowTimer_ = 0;
    skillCooldownA_ = 0; skillCooldownB_ = 0; skillCooldownC_ = 0; skillCooldownD_ = 0;
    passiveCounterA_ = 0; passiveCounterB_ = 0; passiveCounterC_ = 0;
    ownerIndex_ = -1; hostIndex_ = -1; moving_ = false; attacking_ = false;
    passiveFlagA_ = false; passiveFlagB_ = false; passiveFlagC_ = false;
    temporaryInvulnerable_ = false;
    CleanupSummon();
    DeleteOldProjectiles();
}

void LegacyCharacter::ProcessAI(const UpdateInput& input)
{
    if (legacyType_ == LegacyCharacterType::Npc || legacyType_ == LegacyCharacterType::Object)
    {
        aiState_ = 0; velocityX_ = 0; velocityY_ = 0;
    }
    else if (legacyType_ == LegacyCharacterType::Monster)
    {
        aiState_ = input.nearbyEnemyCount > 0 ? 1 : 0;
        velocityX_ = aiState_ == 1 ? 1 : 0; velocityY_ = 0;
    }
    else if (legacyType_ == LegacyCharacterType::BossMonster)
    {
        aiState_ = input.nearbyEnemyCount > 1 ? 2 : 0;
        velocityX_ = aiState_ == 2 ? 1 : 0; velocityY_ = aiState_ == 2 ? 1 : 0;
    }
    else if (legacyType_ == LegacyCharacterType::Summon)
    {
        aiState_ = ownerIndex_ >= 0 ? 3 : 0;
        velocityX_ = aiState_ == 3 ? 1 : 0; velocityY_ = 0;
    }
}

void LegacyCharacter::UpdateSkillTimers()
{
    if (skillCooldownA_ > 0) { --skillCooldownA_; }
    if (skillCooldownB_ > 0) { --skillCooldownB_; }
    if (skillCooldownC_ > 0) { --skillCooldownC_; }
    if (skillCooldownD_ > 0) { --skillCooldownD_; }
    if (attackDelay_ > 0) { --attackDelay_; }
}

void LegacyCharacter::UpdateStatusTimers()
{
    if (poisonTimer_ > 0) { --poisonTimer_; }
    if (stunTimer_ > 0) { --stunTimer_; }
    if (burnTimer_ > 0) { --burnTimer_; }
    if (shieldTimer_ > 0) { --shieldTimer_; }
    if (slowTimer_ > 0) { --slowTimer_; }
}

void LegacyCharacter::UpdateLegacyEffectTriggers()
{
    if (passiveFlagA_ && skillCooldownA_ == 0) { passiveFlagA_ = false; }
    if (passiveFlagB_ && health_ < 3)
    {
        temporaryInvulnerable_ = true;
        passiveFlagB_ = false;
    }
}

void LegacyCharacter::ApplyPassiveEffects(LegacyPassiveMoment moment, int value)
{
    // Legacy if-chain: timing, type, skill state, and health conditions are mixed.
    if (moment == LegacyPassiveMoment::BeforeAttack && passiveFlagA_) { ++passiveCounterA_; }
    if (moment == LegacyPassiveMoment::AfterAttack && legacyType_ == LegacyCharacterType::Monster) { ++passiveCounterB_; }
    if (moment == LegacyPassiveMoment::AfterAttack && legacyType_ == LegacyCharacterType::BossMonster)
    {
        ++passiveCounterB_;
        passiveCounterC_ += health_ < 5 ? 1 : 0;
    }
    if (moment == LegacyPassiveMoment::OnDamaged && value > 0 && shieldTimer_ <= 0) { passiveFlagB_ = true; }
    if (moment == LegacyPassiveMoment::BeforeDeath && legacyType_ == LegacyCharacterType::Summon) { CleanupSummon(); }
    if (moment == LegacyPassiveMoment::BeforeDeath && legacyType_ == LegacyCharacterType::Object) { CleanupSummon(); }
    if (moment == LegacyPassiveMoment::PerFrame && poisonTimer_ > 0) { ++passiveCounterC_; }
}

void LegacyCharacter::CreateProjectile(int targetIndex, int skillId)
{
    if (skillId == 1) { CreateStraightProjectile(targetIndex); }
    else if (skillId == 2) { CreateMultiProjectile(targetIndex); }
    else if (skillId == 3) { CreateDirectionalProjectile(targetIndex, 1, 0); }
    else { CreateHomingProjectile(targetIndex); }
}

void LegacyCharacter::CreateStraightProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    LegacyProjectile* projectile = new LegacyProjectile();
    projectile->ownerId = id_; projectile->x = x_; projectile->y = y_;
    projectile->directionX = target != nullptr && target->x_ >= x_ ? 1 : -1;
    projectile->directionY = 0; projectile->lifeTime = 2; projectile->homing = false;
    projectiles_.push_back(projectile);
}

void LegacyCharacter::CreateMultiProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    const int count = legacyType_ == LegacyCharacterType::BossMonster ? 3 : 2;
    for (int i = 0; i < count; ++i)
    {
        LegacyProjectile* projectile = new LegacyProjectile();
        projectile->ownerId = id_; projectile->x = x_ + i; projectile->y = y_;
        projectile->directionX = target != nullptr && target->x_ >= x_ ? 1 : -1;
        projectile->directionY = i - 1; projectile->lifeTime = 2 + i; projectile->homing = false;
        projectiles_.push_back(projectile);
    }
}

void LegacyCharacter::CreateDirectionalProjectile(int, int directionX, int directionY)
{
    LegacyProjectile* projectile = new LegacyProjectile();
    projectile->ownerId = id_; projectile->x = x_; projectile->y = y_;
    projectile->directionX = directionX; projectile->directionY = directionY;
    projectile->lifeTime = legacyType_ == LegacyCharacterType::BossMonster ? 3 : 2;
    projectiles_.push_back(projectile);
}

void LegacyCharacter::CreateHomingProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    LegacyProjectile* projectile = new LegacyProjectile();
    projectile->ownerId = id_; projectile->x = x_; projectile->y = y_;
    projectile->directionX = target != nullptr ? DirectionTo(target) : 0;
    projectile->directionY = target != nullptr && target->y_ >= y_ ? 1 : -1;
    projectile->lifeTime = 3; projectile->homing = true;
    projectiles_.push_back(projectile);
}

void LegacyCharacter::DeleteOldProjectiles()
{
    for (std::size_t i = 0; i < projectiles_.size();)
    {
        LegacyProjectile* projectile = projectiles_[i];
        if (projectile == nullptr || --projectile->lifeTime <= 0)
        {
            delete projectile;
            projectiles_.erase(projectiles_.begin() + static_cast<std::ptrdiff_t>(i));
        }
        else
        {
            ++i;
        }
    }
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
    case PassiveTiming::OneShot: energy_ += 1; EmitEvent("Passive:OneShot", energy_); break;
    case PassiveTiming::Passive: energy_ += 1; EmitEvent("Passive:Passive", energy_); break;
    case PassiveTiming::PostPassive: EmitEvent("Passive:PostPassive", energy_); break;
    case PassiveTiming::Dying: EmitEvent("Passive:Dying", energy_); break;
    default: break;
    }
}

void LegacyCharacter::UpdateEffects()
{
    if (effectTicks_ <= 0) { return; }
    --effectTicks_;
    if (effectTicks_ == 0) { EmitEvent("EffectExpired", 0); }
}

void LegacyCharacter::FireProjectile(int nearbyEnemyCount)
{
    const int projectileCount = ProjectileCountForKind(nearbyEnemyCount);
    if (projectileCount > 0) { EmitEvent("ProjectileSpawned", projectileCount); }
}

void LegacyCharacter::TryReturnRequest(bool requested)
{
    if (requested) { EmitEvent("ReturnRequestChecked", 1); }
}

void LegacyCharacter::PlayLegacySound(int soundId)
{
    passiveCounterA_ += soundId > 0 ? 1 : 0;
}

void LegacyCharacter::TriggerLegacyEffect(int effectId)
{
    passiveCounterC_ += effectId > 0 ? 1 : 0;
}

int LegacyCharacter::ClampDamage(int rawDamage) const
{
    return std::max(0, rawDamage);
}

int LegacyCharacter::EnergyGainForKind() const
{
    switch (kind_)
    {
    case CharacterKind::Player: return 1;
    case CharacterKind::Monster: return 2;
    case CharacterKind::EliteMonster: return 3;
    default: return 0;
    }
}

int LegacyCharacter::ProjectileCountForKind(int nearbyEnemyCount) const
{
    switch (kind_)
    {
    case CharacterKind::Monster: return std::max(1, nearbyEnemyCount);
    case CharacterKind::EliteMonster: return std::max(1, nearbyEnemyCount + 1);
    case CharacterKind::Summon: return nearbyEnemyCount > 0 ? 1 : 0;
    default: return 1;
    }
}

int LegacyCharacter::DistanceSquaredTo(const LegacyCharacter* other) const
{
    if (other == nullptr) { return 0; }
    const int dx = x_ - other->x_;
    const int dy = y_ - other->y_;
    return dx * dx + dy * dy;
}

int LegacyCharacter::DirectionTo(const LegacyCharacter* other) const
{
    if (other == nullptr) { return 0; }
    return other->x_ >= x_ ? 1 : -1;
}

LegacyCharacterType LegacyCharacter::ToLegacyType(CharacterKind kind) const
{
    switch (kind)
    {
    case CharacterKind::Player: return LegacyCharacterType::Player;
    case CharacterKind::Npc: return LegacyCharacterType::Npc;
    case CharacterKind::Monster: return LegacyCharacterType::Monster;
    case CharacterKind::EliteMonster: return LegacyCharacterType::BossMonster;
    case CharacterKind::Summon: return LegacyCharacterType::Summon;
    case CharacterKind::Object:
    default: return LegacyCharacterType::Object;
    }
}

void LegacyCharacter::Die()
{
    alive_ = false;
    CleanupSummon();
    EmitEvent("Died", id_);
    if (id_ >= 0 && id_ < static_cast<int>(kMaxLegacyCharacterCount) && gCharacters[id_] == this)
    {
        gCharacters[id_] = nullptr;
    }
}

LegacyCharacter& LegacyAt(std::size_t index)
{
    // Legacy compatibility: no explicit validation here.
    return LegacyRegistry[index];
}

LegacyCharacter* LegacyRawAt(int index)
{
    if (index < 0 || index >= static_cast<int>(kMaxLegacyCharacterCount))
    {
        return nullptr;
    }
    return gCharacters[index];
}
}

