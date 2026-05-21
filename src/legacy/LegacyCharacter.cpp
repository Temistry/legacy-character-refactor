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
      silenceTimer_(0),
      rootTimer_(0),
      freezeTimer_(0),
      hasteTimer_(0),
      skillCooldownA_(0),
      skillCooldownB_(0),
      skillCooldownC_(0),
      skillCooldownD_(0),
      skillCooldownE_(0),
      skillCostA_(0),
      skillCostB_(0),
      skillCostC_(0),
      skillCostD_(0),
      skillPowerA_(0),
      skillPowerB_(0),
      skillPowerC_(0),
      skillPowerD_(0),
      passiveCounterA_(0),
      passiveCounterB_(0),
      passiveCounterC_(0),
      passiveCounterD_(0),
      passiveCounterE_(0),
      summonLifeTime_(0),
      ownerIndex_(-1),
      hostIndex_(-1),
      lastRequestedTargetIndex_(-1),
      lastResolvedTargetIndex_(-1),
      lastSkillSlot_(0),
      legacyPathFlags_(0),
      visualEffectHint_(0),
      soundEffectHint_(0),
      alive_(false),
      moving_(false),
      attacking_(false),
      passiveFlagA_(false),
      passiveFlagB_(false),
      passiveFlagC_(false),
      passiveFlagD_(false),
      passiveFlagE_(false),
      pendingProjectile_(false),
      pendingRecall_(false),
      ownerLinkDirty_(false),
      hostLinkDirty_(false),
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
    skillCostA_ = 0;
    skillCostB_ = legacyType_ == LegacyCharacterType::EliteMonster ? 1 : 0;
    skillCostC_ = 1;
    skillCostD_ = 2;
    skillPowerA_ = 1;
    skillPowerB_ = 2;
    skillPowerC_ = 0;
    skillPowerD_ = 0;
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
    UpdateMovementCompatibilityFlags(input);
    UpdateLegacyTargetCache(input.nearbyEnemyCount);
    UpdateLegacyResourceFlags();

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
    NormalizeLegacyTimers();
    UpdateLegacySkillGateFlags();
    UpdateLegacyEffectTriggers();
    UpdateEffects();
    ApplyPassiveEffects(LegacyPassiveMoment::PerFrame, input.frame);
    TriggerPassive(PassiveTiming::Passive);

    if (input.requestProjectile)
    {
        pendingProjectile_ = true;
        FireProjectile(input.nearbyEnemyCount);
    }
    else
    {
        pendingProjectile_ = false;
    }

    pendingRecall_ = input.requestReturn;
    TryReturnRequest(input.requestReturn);
    UpdateLegacyOwnerHostLinks();
    RefreshLegacySummonState();

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
    if (freezeTimer_ > 0 && damage > 0) { passiveCounterD_ += 1; }
    if (temporaryInvulnerable_)
    {
        damage = 0;
        temporaryInvulnerable_ = false;
    }

    ApplyLegacyDamageSideEffects(damage);

    health_ -= damage;
    EmitEvent("Damaged", damage);

    if (health_ <= 0)
    {
        ApplyLegacyNearDeathRules(damage);
        ApplyPassiveEffects(LegacyPassiveMoment::BeforeDeath, damage);
        TriggerPassive(PassiveTiming::Dying);
        Die();
    }
}

void LegacyCharacter::Move(int deltaX, int deltaY)
{
    if (!alive_ || stunTimer_ > 0 || freezeTimer_ > 0 || legacyType_ == LegacyCharacterType::Object)
    {
        moving_ = false;
        return;
    }

    if (rootTimer_ > 0)
    {
        deltaX = 0;
        deltaY = 0;
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

    // Temporary compatibility path: some callers pass raw global indexes.
    targetIndex = ResolveLegacyTargetIndex(targetIndex);
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
    else if (legacyType_ == LegacyCharacterType::EliteMonster)
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

    if (silenceTimer_ > 0)
    {
        passiveCounterE_ += 1;
        return;
    }

    const int slot = ResolveLegacySkillSlot(skillId);
    lastSkillSlot_ = slot;
    targetIndex = ResolveLegacyTargetIndex(targetIndex);

    // Called from many places. The slot state, resource check, projectile path,
    // summon path, and passive state all stay in this one legacy function.
    if (!CanUseLegacySkillSlot(slot))
    {
        return;
    }

    ApplyLegacySkillCost(slot);

    if (skillId == 1 && skillCooldownA_ <= 0)
    {
        StartLegacySkillCooldown(slot);
        passiveFlagA_ = true;
        CreateProjectile(targetIndex, skillId);
    }
    else if (skillId == 2 && skillCooldownB_ <= 0)
    {
        StartLegacySkillCooldown(slot);
        passiveCounterB_ += 1;
        CreateMultiProjectile(targetIndex);
    }
    else if (skillId == 3 && skillCooldownC_ <= 0)
    {
        StartLegacySkillCooldown(slot);
        stunTimer_ = std::max(stunTimer_, 1);
        CreateDirectionalProjectile(targetIndex, 0, 1);
    }
    else if (skillId == 4 && skillCooldownD_ <= 0)
    {
        StartLegacySkillCooldown(slot);
        SummonUnit(targetIndex);
    }
    else if (skillId == 5 && skillCooldownE_ <= 0)
    {
        StartLegacySkillCooldown(slot);
        // Special case for legacy type branch: copied projectile math with a
        // different shape flag instead of a separate strategy object.
        CreateSectorProjectile(targetIndex);
    }
}

void LegacyCharacter::SummonUnit(int targetIndex)
{
    targetIndex = ResolveLegacyTargetIndex(targetIndex);
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
    else if (legacyType_ == LegacyCharacterType::EliteMonster)
    {
        summon_->lifeTime = 3;
        passiveFlagC_ = true;
    }
    else if (legacyType_ == LegacyCharacterType::Summon)
    {
        summon_->lifeTime = 1;
        ownerIndex_ = targetIndex;
        ownerLinkDirty_ = true;
    }
    else
    {
        summon_->lifeTime = 2;
        hostIndex_ = targetIndex;
        hostLinkDirty_ = true;
    }

    summonLifeTime_ = summon_->lifeTime;
    UpdateLegacyOwnerHostLinks();
}

void LegacyCharacter::RecallUnit()
{
    if (summon_ == nullptr) { return; }

    if (legacyType_ == LegacyCharacterType::Summon && ownerIndex_ >= 0)
    {
        summon_->hostId = ownerIndex_;
        ownerLinkDirty_ = false;
    }
    else if (legacyType_ == LegacyCharacterType::Object && hostIndex_ >= 0)
    {
        summon_->hostId = hostIndex_;
        hostLinkDirty_ = false;
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
    ownerLinkDirty_ = false;
    hostLinkDirty_ = false;
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
    silenceTimer_ = 0; rootTimer_ = 0; freezeTimer_ = 0; hasteTimer_ = 0;
    skillCooldownA_ = 0; skillCooldownB_ = 0; skillCooldownC_ = 0; skillCooldownD_ = 0; skillCooldownE_ = 0;
    passiveCounterA_ = 0; passiveCounterB_ = 0; passiveCounterC_ = 0; passiveCounterD_ = 0; passiveCounterE_ = 0;
    ownerIndex_ = -1; hostIndex_ = -1; moving_ = false; attacking_ = false;
    lastRequestedTargetIndex_ = -1; lastResolvedTargetIndex_ = -1; lastSkillSlot_ = 0;
    legacyPathFlags_ = 0; visualEffectHint_ = 0; soundEffectHint_ = 0;
    passiveFlagA_ = false; passiveFlagB_ = false; passiveFlagC_ = false;
    passiveFlagD_ = false; passiveFlagE_ = false;
    pendingProjectile_ = false; pendingRecall_ = false; ownerLinkDirty_ = false; hostLinkDirty_ = false;
    temporaryInvulnerable_ = false;
    CleanupSummon();
    DeleteOldProjectiles();
}

void LegacyCharacter::ProcessAI(const UpdateInput& input)
{
    // Special case for legacy type branch: AI writes movement, target, and
    // state flags directly into Character fields.
    if (legacyType_ == LegacyCharacterType::Npc || legacyType_ == LegacyCharacterType::Object)
    {
        aiState_ = 0; velocityX_ = 0; velocityY_ = 0;
    }
    else if (legacyType_ == LegacyCharacterType::Monster)
    {
        aiState_ = input.nearbyEnemyCount > 0 ? 1 : 0;
        velocityX_ = aiState_ == 1 ? 1 : 0; velocityY_ = 0;
    }
    else if (legacyType_ == LegacyCharacterType::EliteMonster)
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

void LegacyCharacter::UpdateMovementCompatibilityFlags(const UpdateInput& input)
{
    // Temporary compatibility path: input state is converted into old flags
    // before the real update work starts.
    if (input.nearbyEnemyCount > 0)
    {
        legacyPathFlags_ |= 1;
    }
    else
    {
        legacyPathFlags_ &= ~1;
    }

    if (input.requestProjectile)
    {
        legacyPathFlags_ |= 2;
    }
    else
    {
        legacyPathFlags_ &= ~2;
    }

    if (input.requestReturn)
    {
        legacyPathFlags_ |= 4;
    }
    else
    {
        legacyPathFlags_ &= ~4;
    }
}

void LegacyCharacter::UpdateLegacyTargetCache(int candidateIndex)
{
    // Called from many places in old code paths. The cache is not a separate
    // service, so every caller has to know which index form it owns.
    lastRequestedTargetIndex_ = candidateIndex;
    if (candidateIndex >= 0 && candidateIndex < static_cast<int>(kMaxLegacyCharacterCount))
    {
        lastResolvedTargetIndex_ = candidateIndex;
    }
    else
    {
        lastResolvedTargetIndex_ = -1;
    }
}

void LegacyCharacter::UpdateLegacySkillGateFlags()
{
    // These flags represent feature gates that accumulated around cooldowns.
    passiveFlagD_ = skillCooldownA_ > 0 || skillCooldownB_ > 0;
    passiveFlagE_ = skillCooldownC_ > 0 || skillCooldownD_ > 0 || skillCooldownE_ > 0;

    if (legacyType_ == LegacyCharacterType::Monster && passiveFlagD_)
    {
        passiveCounterD_ += 1;
    }
    else if (legacyType_ == LegacyCharacterType::EliteMonster && passiveFlagE_)
    {
        passiveCounterE_ += 1;
    }
}

void LegacyCharacter::UpdateLegacyResourceFlags()
{
    // Magic values stay near gameplay code in this legacy shape.
    if (energy_ >= 3)
    {
        legacyPathFlags_ |= 8;
    }
    else
    {
        legacyPathFlags_ &= ~8;
    }

    if (health_ <= 3 && alive_)
    {
        legacyPathFlags_ |= 16;
    }
    else
    {
        legacyPathFlags_ &= ~16;
    }
}

void LegacyCharacter::NormalizeLegacyTimers()
{
    // Defensive clamp added after several timer fields accumulated.
    poisonTimer_ = std::max(0, poisonTimer_);
    stunTimer_ = std::max(0, stunTimer_);
    burnTimer_ = std::max(0, burnTimer_);
    shieldTimer_ = std::max(0, shieldTimer_);
    slowTimer_ = std::max(0, slowTimer_);
    silenceTimer_ = std::max(0, silenceTimer_);
    rootTimer_ = std::max(0, rootTimer_);
    freezeTimer_ = std::max(0, freezeTimer_);
    hasteTimer_ = std::max(0, hasteTimer_);
}

void LegacyCharacter::UpdateLegacyOwnerHostLinks()
{
    // Owner and host state sits next to combat state. This is intentionally
    // awkward so the refactored boundary can be compared against it.
    if (summon_ == nullptr)
    {
        ownerLinkDirty_ = false;
        hostLinkDirty_ = false;
        return;
    }

    if (ownerLinkDirty_ && ownerIndex_ >= 0)
    {
        summon_->ownerId = ownerIndex_;
        ownerLinkDirty_ = false;
    }

    if (hostLinkDirty_ && hostIndex_ >= 0)
    {
        summon_->hostId = hostIndex_;
        hostLinkDirty_ = false;
    }
}

void LegacyCharacter::RefreshLegacySummonState()
{
    // Special case: similar lifetime logic appears in more than one place.
    if (summon_ == nullptr || !summon_->active)
    {
        summonLifeTime_ = 0;
        return;
    }

    summonLifeTime_ = summon_->lifeTime;
    if (legacyType_ == LegacyCharacterType::EliteMonster && summonLifeTime_ == 1)
    {
        passiveFlagC_ = true;
    }
}

void LegacyCharacter::UpdateSkillTimers()
{
    if (skillCooldownA_ > 0) { --skillCooldownA_; }
    if (skillCooldownB_ > 0) { --skillCooldownB_; }
    if (skillCooldownC_ > 0) { --skillCooldownC_; }
    if (skillCooldownD_ > 0) { --skillCooldownD_; }
    if (skillCooldownE_ > 0) { --skillCooldownE_; }
    if (attackDelay_ > 0) { --attackDelay_; }
}

void LegacyCharacter::UpdateStatusTimers()
{
    if (poisonTimer_ > 0) { --poisonTimer_; }
    if (stunTimer_ > 0) { --stunTimer_; }
    if (burnTimer_ > 0) { --burnTimer_; }
    if (shieldTimer_ > 0) { --shieldTimer_; }
    if (slowTimer_ > 0) { --slowTimer_; }
    if (silenceTimer_ > 0) { --silenceTimer_; }
    if (rootTimer_ > 0) { --rootTimer_; }
    if (freezeTimer_ > 0) { --freezeTimer_; }
    if (hasteTimer_ > 0) { --hasteTimer_; }
}

void LegacyCharacter::UpdateLegacyEffectTriggers()
{
    if (passiveFlagA_ && skillCooldownA_ == 0) { passiveFlagA_ = false; }
    if (passiveFlagB_ && health_ < 3)
    {
        temporaryInvulnerable_ = true;
        passiveFlagB_ = false;
    }

    // Visual and sound hints are kept as plain integers in the legacy class.
    // They do not emit public events in this toy sample.
    if (burnTimer_ > 0)
    {
        visualEffectHint_ = 1;
    }
    else if (poisonTimer_ > 0)
    {
        visualEffectHint_ = 2;
    }
    else if (shieldTimer_ > 0)
    {
        visualEffectHint_ = 3;
    }
    else
    {
        visualEffectHint_ = 0;
    }

    if (attacking_)
    {
        soundEffectHint_ = 1;
    }
    else if (hitStopTimer_ > 0)
    {
        soundEffectHint_ = 2;
    }
    else
    {
        soundEffectHint_ = 0;
    }
}

void LegacyCharacter::ApplyPassiveEffects(LegacyPassiveMoment moment, int value)
{
    // Legacy if-chain: timing, type, skill state, and health conditions are mixed.
    if (moment == LegacyPassiveMoment::BeforeAttack && passiveFlagA_) { ++passiveCounterA_; }
    if (moment == LegacyPassiveMoment::AfterAttack && legacyType_ == LegacyCharacterType::Monster) { ++passiveCounterB_; }
    if (moment == LegacyPassiveMoment::AfterAttack && legacyType_ == LegacyCharacterType::EliteMonster)
    {
        ++passiveCounterB_;
        passiveCounterC_ += health_ < 5 ? 1 : 0;
    }
    if (moment == LegacyPassiveMoment::OnDamaged && value > 0 && shieldTimer_ <= 0) { passiveFlagB_ = true; }
    if (moment == LegacyPassiveMoment::BeforeDeath && legacyType_ == LegacyCharacterType::Summon) { CleanupSummon(); }
    if (moment == LegacyPassiveMoment::BeforeDeath && legacyType_ == LegacyCharacterType::Object) { CleanupSummon(); }
    if (moment == LegacyPassiveMoment::PerFrame && poisonTimer_ > 0) { ++passiveCounterC_; }
    if (moment == LegacyPassiveMoment::PerFrame && burnTimer_ > 0 && legacyType_ != LegacyCharacterType::Object) { ++passiveCounterD_; }
    if (moment == LegacyPassiveMoment::OnDamaged && passiveFlagD_) { passiveCounterE_ += value > 0 ? 1 : 0; }
    if (moment == LegacyPassiveMoment::BeforeAttack && skillCooldownE_ > 0) { passiveCounterD_ += 1; }
    if (moment == LegacyPassiveMoment::AfterAttack && pendingProjectile_) { passiveCounterE_ += 1; }
    if (moment == LegacyPassiveMoment::BeforeDeath && pendingRecall_) { pendingRecall_ = false; }
}

void LegacyCharacter::ApplyLegacyDamageSideEffects(int damage)
{
    // Called from damage code and kept close to health mutation in legacy code.
    if (damage <= 0)
    {
        return;
    }

    hitStopTimer_ = std::max(hitStopTimer_, 1);
    if (legacyType_ == LegacyCharacterType::Player)
    {
        visualEffectHint_ = 10;
    }
    else if (legacyType_ == LegacyCharacterType::Monster)
    {
        visualEffectHint_ = 20;
    }
    else if (legacyType_ == LegacyCharacterType::EliteMonster)
    {
        visualEffectHint_ = 30;
        passiveCounterD_ += 1;
    }
    else if (legacyType_ == LegacyCharacterType::Summon)
    {
        visualEffectHint_ = 40;
    }
    else
    {
        visualEffectHint_ = 0;
    }
}

void LegacyCharacter::ApplyLegacyNearDeathRules(int damage)
{
    // Special case for legacy type branch. These flags model logic that was
    // added near death handling instead of being moved to a passive pipeline.
    if (damage <= 0)
    {
        return;
    }

    if (legacyType_ == LegacyCharacterType::Monster)
    {
        passiveFlagC_ = true;
    }
    else if (legacyType_ == LegacyCharacterType::EliteMonster)
    {
        passiveFlagC_ = true;
        passiveCounterE_ += 1;
    }
    else if (legacyType_ == LegacyCharacterType::Summon)
    {
        ownerLinkDirty_ = false;
    }
    else if (legacyType_ == LegacyCharacterType::Object)
    {
        hostLinkDirty_ = false;
    }
}

void LegacyCharacter::CreateProjectile(int targetIndex, int skillId)
{
    if (skillId == 1) { CreateStraightProjectile(targetIndex); }
    else if (skillId == 2) { CreateMultiProjectile(targetIndex); }
    else if (skillId == 3) { CreateDirectionalProjectile(targetIndex, 1, 0); }
    else if (skillId == 5) { CreateSectorProjectile(targetIndex); }
    else { CreateHomingProjectile(targetIndex); }
}

void LegacyCharacter::CreateStraightProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    LegacyProjectile* projectile = AllocateProjectile(id_, x_, y_);
    projectile->directionX = target != nullptr && target->x_ >= x_ ? 1 : -1;
    projectile->directionY = 0; projectile->lifeTime = ChooseProjectileLifeTime(1); projectile->homing = false;
    PushProjectileWithLegacyDefaults(projectile);
}

void LegacyCharacter::CreateMultiProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    const int count = legacyType_ == LegacyCharacterType::EliteMonster ? 3 : 2;
    for (int i = 0; i < count; ++i)
    {
        LegacyProjectile* projectile = AllocateProjectile(id_, x_ + i, y_);
        projectile->directionX = target != nullptr && target->x_ >= x_ ? 1 : -1;
        projectile->directionY = i - 1; projectile->lifeTime = ChooseProjectileLifeTime(2) + i; projectile->homing = false;
        PushProjectileWithLegacyDefaults(projectile);
    }
}

void LegacyCharacter::CreateDirectionalProjectile(int, int directionX, int directionY)
{
    LegacyProjectile* projectile = AllocateProjectile(id_, x_, y_);
    projectile->directionX = directionX; projectile->directionY = directionY;
    projectile->lifeTime = ChooseProjectileLifeTime(3);
    PushProjectileWithLegacyDefaults(projectile);
}

void LegacyCharacter::CreateHomingProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    LegacyProjectile* projectile = AllocateProjectile(id_, x_, y_);
    projectile->directionX = target != nullptr ? DirectionTo(target) : 0;
    projectile->directionY = target != nullptr && target->y_ >= y_ ? 1 : -1;
    projectile->lifeTime = ChooseProjectileLifeTime(4); projectile->homing = true;
    PushProjectileWithLegacyDefaults(projectile);
}

void LegacyCharacter::CreateSectorProjectile(int targetIndex)
{
    LegacyCharacter* target = LegacyRawAt(targetIndex);
    const int centerDirection = target != nullptr ? DirectionTo(target) : 1;

    for (int i = -1; i <= 1; ++i)
    {
        LegacyProjectile* projectile = AllocateProjectile(id_, x_ + i, y_);
        projectile->directionX = centerDirection;
        projectile->directionY = i;
        projectile->lifeTime = ChooseProjectileLifeTime(5);
        projectile->homing = false;
        PushProjectileWithLegacyDefaults(projectile);
    }
}

LegacyProjectile* LegacyCharacter::AllocateProjectile(int ownerId, int startX, int startY)
{
    // Allocation policy is scattered in legacy code. This helper still returns
    // raw ownership and does not solve the lifetime problem.
    LegacyProjectile* projectile = new LegacyProjectile();
    projectile->ownerId = ownerId;
    projectile->x = startX;
    projectile->y = startY;
    return projectile;
}

void LegacyCharacter::PushProjectileWithLegacyDefaults(LegacyProjectile* projectile)
{
    if (projectile == nullptr)
    {
        return;
    }

    if (legacyType_ == LegacyCharacterType::Object)
    {
        delete projectile;
        return;
    }

    if (projectile->lifeTime <= 0)
    {
        projectile->lifeTime = 1;
    }

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

void LegacyCharacter::DeleteProjectilesOwnedBy(int ownerId)
{
    // Temporary cleanup path used by summon and owner branches.
    for (std::size_t i = 0; i < projectiles_.size();)
    {
        LegacyProjectile* projectile = projectiles_[i];
        if (projectile != nullptr && projectile->ownerId == ownerId)
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

bool LegacyCharacter::CanUseLegacySkillSlot(int slot) const
{
    if (slot <= 0)
    {
        return false;
    }

    if (slot == 1) { return skillCooldownA_ <= 0 && energy_ >= skillCostA_; }
    if (slot == 2) { return skillCooldownB_ <= 0 && energy_ >= skillCostB_; }
    if (slot == 3) { return skillCooldownC_ <= 0 && energy_ >= skillCostC_; }
    if (slot == 4) { return skillCooldownD_ <= 0 && energy_ >= skillCostD_; }
    if (slot == 5) { return skillCooldownE_ <= 0 && energy_ >= 0; }
    return false;
}

void LegacyCharacter::StartLegacySkillCooldown(int slot)
{
    // Magic cooldown values stayed close to skill id branches.
    if (slot == 1)
    {
        skillCooldownA_ = 3;
    }
    else if (slot == 2)
    {
        skillCooldownB_ = legacyType_ == LegacyCharacterType::EliteMonster ? 4 : 3;
    }
    else if (slot == 3)
    {
        skillCooldownC_ = 5;
    }
    else if (slot == 4)
    {
        skillCooldownD_ = 6;
    }
    else if (slot == 5)
    {
        skillCooldownE_ = legacyType_ == LegacyCharacterType::EliteMonster ? 5 : 4;
    }
}

void LegacyCharacter::ApplyLegacySkillCost(int slot)
{
    const int cost = LegacySkillCostForSlot(slot);
    energy_ = std::max(0, energy_ - cost);

    // Power values are kept in the Character class even though they are data.
    if (LegacySkillPowerForSlot(slot) > 1)
    {
        passiveCounterD_ += 1;
    }
}

int LegacyCharacter::ResolveLegacySkillSlot(int skillId) const
{
    if (skillId == 1) { return 1; }
    if (skillId == 2) { return 2; }
    if (skillId == 3) { return 3; }
    if (skillId == 4) { return 4; }
    if (skillId == 5) { return 5; }
    return 0;
}

int LegacyCharacter::LegacySkillCostForSlot(int slot) const
{
    if (slot == 1) { return skillCostA_; }
    if (slot == 2) { return skillCostB_; }
    if (slot == 3) { return skillCostC_; }
    if (slot == 4) { return skillCostD_; }
    return 0;
}

int LegacyCharacter::LegacySkillPowerForSlot(int slot) const
{
    if (slot == 1) { return skillPowerA_; }
    if (slot == 2) { return skillPowerB_; }
    if (slot == 3) { return skillPowerC_; }
    if (slot == 4) { return skillPowerD_; }
    return 0;
}

int LegacyCharacter::ChooseProjectileLifeTime(int skillId) const
{
    // Special case values are intentionally toy values.
    if (skillId == 2 && legacyType_ == LegacyCharacterType::EliteMonster) { return 3; }
    if (skillId == 4) { return 3; }
    if (skillId == 5) { return legacyType_ == LegacyCharacterType::EliteMonster ? 4 : 2; }
    return 2;
}

int LegacyCharacter::ResolveLegacyTargetIndex(int requestedIndex) const
{
    if (requestedIndex >= 0 && requestedIndex < static_cast<int>(kMaxLegacyCharacterCount))
    {
        return requestedIndex;
    }

    if (lastResolvedTargetIndex_ >= 0)
    {
        return lastResolvedTargetIndex_;
    }

    return targetIndex_;
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
    case CharacterKind::EliteMonster: return LegacyCharacterType::EliteMonster;
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

