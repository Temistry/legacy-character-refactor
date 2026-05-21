#pragma once

#include "CharacterTypes.h"

namespace portfolio
{
namespace CharacterMath
{
int ClampDamage(int rawDamage);
int ClampDuration(int durationFrames);
int EnergyGainForKind(CharacterKind kind);
int ProjectileCountForKind(CharacterKind kind, int nearbyEnemyCount);
int DistanceSquared(int ax, int ay, int bx, int by);
Vec2 DirectionStep(Vec2 from, Vec2 to);
CharacterEvent BuildEvent(const std::string& name, int value);

CharacterStateSnapshot BuildSnapshot(
    int id,
    CharacterKind kind,
    int health,
    int energy,
    int activeEffectCount,
    bool alive);
}
}
