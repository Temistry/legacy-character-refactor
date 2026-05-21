#include "CharacterMath.h"

#include <algorithm>

namespace portfolio
{
namespace CharacterMath
{
int ClampDamage(int rawDamage)
{
    return std::max(0, rawDamage);
}

int ClampDuration(int durationFrames)
{
    return std::max(0, durationFrames);
}

int EnergyGainForKind(CharacterKind kind)
{
    switch (kind)
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

int ProjectileCountForKind(CharacterKind kind, int nearbyEnemyCount)
{
    switch (kind)
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

int DistanceSquared(int ax, int ay, int bx, int by)
{
    const int dx = ax - bx;
    const int dy = ay - by;
    return dx * dx + dy * dy;
}

Vec2 DirectionStep(Vec2 from, Vec2 to)
{
    Vec2 step;
    step.x = to.x > from.x ? 1 : (to.x < from.x ? -1 : 0);
    step.y = to.y > from.y ? 1 : (to.y < from.y ? -1 : 0);
    return step;
}

CharacterEvent BuildEvent(const std::string& name, int value)
{
    CharacterEvent event;
    event.name = name;
    event.value = value;
    return event;
}

CharacterStateSnapshot BuildSnapshot(
    int id,
    CharacterKind kind,
    int health,
    int energy,
    int activeEffectCount,
    bool alive)
{
    CharacterStateSnapshot snapshot;
    snapshot.id = id;
    snapshot.kind = kind;
    snapshot.health = health;
    snapshot.energy = energy;
    snapshot.activeEffectCount = activeEffectCount;
    snapshot.alive = alive;
    return snapshot;
}
}
}
