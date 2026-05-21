#pragma once

#include "../refactored/CharacterTypes.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace portfolio
{
enum class LegacyCharacterType
{
    Player,
    Npc,
    Monster,
    EliteMonster,
    Summon,
    Object
};

enum class LegacyPassiveMoment
{
    BeforeAttack,
    AfterAttack,
    OnDamaged,
    BeforeDeath,
    PerFrame
};

struct LegacyProjectile
{
    int ownerId = -1;
    int x = 0;
    int y = 0;
    int directionX = 0;
    int directionY = 0;
    int lifeTime = 0;
    bool homing = false;
};

struct LegacySummonData
{
    int ownerId = -1;
    int hostId = -1;
    int lifeTime = 0;
    bool active = false;
};

class LegacyCharacter
{
public:
    LegacyCharacter();
    ~LegacyCharacter();

    void Initialize(int id, CharacterKind kind, int health);
    void Update(const UpdateInput& input);
    void ApplyDamage(int rawDamage);

    void Move(int deltaX, int deltaY);
    void Attack(int targetIndex);
    void CastSkill(int skillId, int targetIndex);
    void SummonUnit(int targetIndex);
    void RecallUnit();
    void CleanupSummon();

    CharacterStateSnapshot Snapshot() const;
    const std::vector<CharacterEvent>& Events() const;
    void ClearEvents();

private:
    void ResetLegacyState();
    void ProcessAI(const UpdateInput& input);
    void UpdateSkillTimers();
    void UpdateStatusTimers();
    void UpdateMovementCompatibilityFlags(const UpdateInput& input);
    void UpdateLegacyTargetCache(int candidateIndex);
    void UpdateLegacySkillGateFlags();
    void UpdateLegacyResourceFlags();
    void NormalizeLegacyTimers();
    void UpdateLegacyOwnerHostLinks();
    void RefreshLegacySummonState();
    void UpdateLegacyEffectTriggers();
    void ApplyPassiveEffects(LegacyPassiveMoment moment, int value);
    void ApplyLegacyDamageSideEffects(int damage);
    void ApplyLegacyNearDeathRules(int damage);
    void CreateProjectile(int targetIndex, int skillId);
    void CreateStraightProjectile(int targetIndex);
    void CreateMultiProjectile(int targetIndex);
    void CreateDirectionalProjectile(int targetIndex, int directionX, int directionY);
    void CreateHomingProjectile(int targetIndex);
    void CreateSectorProjectile(int targetIndex);
    LegacyProjectile* AllocateProjectile(int ownerId, int startX, int startY);
    void PushProjectileWithLegacyDefaults(LegacyProjectile* projectile);
    void DeleteOldProjectiles();
    void DeleteProjectilesOwnedBy(int ownerId);
    void EmitEvent(const std::string& name, int value);
    void TriggerPassive(PassiveTiming timing);
    void UpdateEffects();
    void FireProjectile(int nearbyEnemyCount);
    void TryReturnRequest(bool requested);
    void PlayLegacySound(int soundId);
    void TriggerLegacyEffect(int effectId);
    bool CanUseLegacySkillSlot(int slot) const;
    void StartLegacySkillCooldown(int slot);
    void ApplyLegacySkillCost(int slot);
    int ResolveLegacySkillSlot(int skillId) const;
    int LegacySkillCostForSlot(int slot) const;
    int LegacySkillPowerForSlot(int slot) const;
    int ChooseProjectileLifeTime(int skillId) const;
    int ResolveLegacyTargetIndex(int requestedIndex) const;
    int ClampDamage(int rawDamage) const;
    int EnergyGainForKind() const;
    int ProjectileCountForKind(int nearbyEnemyCount) const;
    int DistanceSquaredTo(const LegacyCharacter* other) const;
    int DirectionTo(const LegacyCharacter* other) const;
    LegacyCharacterType ToLegacyType(CharacterKind kind) const;
    void Die();

    int id_;
    CharacterKind kind_;
    LegacyCharacterType legacyType_;
    int health_;
    int energy_;
    int x_;
    int y_;
    int velocityX_;
    int velocityY_;
    int aiState_;
    int targetIndex_;
    int attackDelay_;
    int hitStopTimer_;
    int effectTicks_;
    int poisonTimer_;
    int stunTimer_;
    int burnTimer_;
    int shieldTimer_;
    int slowTimer_;
    int silenceTimer_;
    int rootTimer_;
    int freezeTimer_;
    int hasteTimer_;
    int skillCooldownA_;
    int skillCooldownB_;
    int skillCooldownC_;
    int skillCooldownD_;
    int skillCooldownE_;
    int skillCostA_;
    int skillCostB_;
    int skillCostC_;
    int skillCostD_;
    int skillPowerA_;
    int skillPowerB_;
    int skillPowerC_;
    int skillPowerD_;
    int passiveCounterA_;
    int passiveCounterB_;
    int passiveCounterC_;
    int passiveCounterD_;
    int passiveCounterE_;
    int summonLifeTime_;
    int ownerIndex_;
    int hostIndex_;
    int lastRequestedTargetIndex_;
    int lastResolvedTargetIndex_;
    int lastSkillSlot_;
    int legacyPathFlags_;
    int visualEffectHint_;
    int soundEffectHint_;
    bool alive_;
    bool moving_;
    bool attacking_;
    bool passiveFlagA_;
    bool passiveFlagB_;
    bool passiveFlagC_;
    bool passiveFlagD_;
    bool passiveFlagE_;
    bool pendingProjectile_;
    bool pendingRecall_;
    bool ownerLinkDirty_;
    bool hostLinkDirty_;
    bool temporaryInvulnerable_;
    LegacySummonData* summon_;
    std::vector<LegacyProjectile*> projectiles_;
    std::vector<CharacterEvent> events_;
};

const std::size_t kLegacyRegistrySize = 8;
const std::size_t kMaxLegacyCharacterCount = 16;

extern std::array<LegacyCharacter, kLegacyRegistrySize> LegacyRegistry;
extern LegacyCharacter* gCharacters[kMaxLegacyCharacterCount];

LegacyCharacter& LegacyAt(std::size_t index);
LegacyCharacter* LegacyRawAt(int index);
}

