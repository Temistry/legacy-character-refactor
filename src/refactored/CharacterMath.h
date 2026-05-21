#pragma once

#include "CharacterTypes.h"

namespace portfolio
{
namespace CharacterMath
{
int ClampDamage(int rawDamage);
int EnergyGainForKind(CharacterKind kind);
int ProjectileCountForKind(CharacterKind kind, int nearbyEnemyCount);
int DistanceSquared(int ax, int ay, int bx, int by);

CharacterStateSnapshot BuildSnapshot(
    int id,
    CharacterKind kind,
    int health,
    int energy,
    int activeEffectCount,
    bool alive);
}
}

