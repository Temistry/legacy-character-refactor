#pragma once

#include "../refactored/CharacterTypes.h"

#include <array>
#include <cstddef>
#include <vector>

namespace portfolio
{
class LegacyCharacter
{
public:
    LegacyCharacter();

    void Initialize(int id, CharacterKind kind, int health);
    void Update(const UpdateInput& input);
    void ApplyDamage(int rawDamage);

    CharacterStateSnapshot Snapshot() const;
    const std::vector<CharacterEvent>& Events() const;
    void ClearEvents();

private:
    void EmitEvent(const std::string& name, int value);
    void TriggerPassive(PassiveTiming timing);
    void UpdateEffects();
    void FireProjectile(int nearbyEnemyCount);
    void TryReturnRequest(bool requested);
    int ClampDamage(int rawDamage) const;
    int EnergyGainForKind() const;
    int ProjectileCountForKind(int nearbyEnemyCount) const;
    void Die();

    int id_;
    CharacterKind kind_;
    int health_;
    int energy_;
    int effectTicks_;
    bool alive_;
    std::vector<CharacterEvent> events_;
};

const std::size_t kLegacyRegistrySize = 8;
extern std::array<LegacyCharacter, kLegacyRegistrySize> LegacyRegistry;

LegacyCharacter& LegacyAt(std::size_t index);
}

