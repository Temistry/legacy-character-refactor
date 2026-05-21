#pragma once

#include <string>

namespace portfolio
{
struct Vec2
{
    int x = 0;
    int y = 0;
};

enum class CharacterKind
{
    Player,
    Npc,
    Monster,
    Summon,
    Object,
    EliteMonster
};

enum class PassiveTiming
{
    OneShot,
    Passive,
    PostPassive,
    Dying
};

struct UpdateInput
{
    int frame = 0;
    int nearbyEnemyCount = 0;
    bool requestProjectile = false;
    bool requestReturn = false;
    Vec2 targetPosition;
};

struct DamageInput
{
    int rawDamage = 0;
    bool ignoreShield = false;
};

struct LifecycleTracePoint
{
    std::string stage;
    int frame = 0;
};

struct CharacterEvent
{
    std::string name;
    int value = 0;

    bool operator==(const CharacterEvent& other) const
    {
        return name == other.name && value == other.value;
    }
};

struct CharacterStateSnapshot
{
    int id = -1;
    CharacterKind kind = CharacterKind::Object;
    int health = 0;
    int energy = 0;
    int activeEffectCount = 0;
    bool alive = false;

    bool operator==(const CharacterStateSnapshot& other) const
    {
        return id == other.id &&
               kind == other.kind &&
               health == other.health &&
               energy == other.energy &&
               activeEffectCount == other.activeEffectCount &&
               alive == other.alive;
    }
};
}
