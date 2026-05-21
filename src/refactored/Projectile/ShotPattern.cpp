#include "ShotPattern.h"

#include "../Character.h"
#include "../CharacterMath.h"

namespace portfolio
{
namespace
{
void EmitProjectileEvent(int count, std::vector<CharacterEvent>& events)
{
    if (count <= 0)
    {
        return;
    }

    CharacterEvent event;
    event.name = "ProjectileSpawned";
    event.value = count;
    events.push_back(event);
}

class StraightPattern : public ShotPattern
{
public:
    ShotPlan BuildPlan(const ShotRequest& request) const override
    {
        ShotPlan plan;
        ShotSpawn spawn;
        spawn.position = request.origin;
        spawn.direction = CharacterMath::DirectionStep(request.origin, request.target);
        spawn.lifeTime = 2;
        plan.spawns.push_back(spawn);
        return plan;
    }

    void Emit(const Character&, const UpdateInput&, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(1, events);
    }
};

class MultiShotPattern : public ShotPattern
{
public:
    ShotPlan BuildPlan(const ShotRequest& request) const override
    {
        ShotPlan plan;
        const int count = CharacterMath::ProjectileCountForKind(CharacterKind::Monster, request.nearbyEnemyCount);
        for (int i = 0; i < count; ++i)
        {
            ShotSpawn spawn;
            spawn.position = request.origin;
            spawn.position.x += i;
            spawn.direction = CharacterMath::DirectionStep(request.origin, request.target);
            spawn.direction.y += i - 1;
            spawn.lifeTime = 2 + i;
            plan.spawns.push_back(spawn);
        }
        return plan;
    }

    void Emit(const Character&, const UpdateInput& input, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(CharacterMath::ProjectileCountForKind(CharacterKind::Monster, input.nearbyEnemyCount), events);
    }
};

class DirectionalPattern : public ShotPattern
{
public:
    ShotPlan BuildPlan(const ShotRequest& request) const override
    {
        ShotPlan plan;
        ShotSpawn spawn;
        spawn.position = request.origin;
        spawn.direction = CharacterMath::DirectionStep(request.origin, request.target);
        spawn.lifeTime = 2;
        plan.spawns.push_back(spawn);
        return plan;
    }

    void Emit(const Character&, const UpdateInput&, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(1, events);
    }
};

class SectorPattern : public ShotPattern
{
public:
    ShotPlan BuildPlan(const ShotRequest& request) const override
    {
        ShotPlan plan;
        const int count = CharacterMath::ProjectileCountForKind(CharacterKind::EliteMonster, request.nearbyEnemyCount);
        for (int i = 0; i < count; ++i)
        {
            ShotSpawn spawn;
            spawn.position = request.origin;
            spawn.direction.x = i - 1;
            spawn.direction.y = 1;
            spawn.lifeTime = 3;
            plan.spawns.push_back(spawn);
        }
        return plan;
    }

    void Emit(const Character&, const UpdateInput& input, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(CharacterMath::ProjectileCountForKind(CharacterKind::EliteMonster, input.nearbyEnemyCount), events);
    }
};

class HomingPattern : public ShotPattern
{
public:
    ShotPlan BuildPlan(const ShotRequest& request) const override
    {
        ShotPlan plan;
        const int count = CharacterMath::ProjectileCountForKind(CharacterKind::Summon, request.nearbyEnemyCount);
        for (int i = 0; i < count; ++i)
        {
            ShotSpawn spawn;
            spawn.position = request.origin;
            spawn.direction = CharacterMath::DirectionStep(request.origin, request.target);
            spawn.lifeTime = 3;
            spawn.homing = true;
            plan.spawns.push_back(spawn);
        }
        return plan;
    }

    void Emit(const Character&, const UpdateInput& input, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(CharacterMath::ProjectileCountForKind(CharacterKind::Summon, input.nearbyEnemyCount), events);
    }
};
}

std::unique_ptr<ShotPattern> CreateStraightShotPattern()
{
    return std::unique_ptr<ShotPattern>(new StraightPattern());
}

std::unique_ptr<ShotPattern> CreateMultiShotPattern()
{
    return std::unique_ptr<ShotPattern>(new MultiShotPattern());
}

std::unique_ptr<ShotPattern> CreateDirectionalShotPattern()
{
    return std::unique_ptr<ShotPattern>(new DirectionalPattern());
}

std::unique_ptr<ShotPattern> CreateSectorShotPattern()
{
    return std::unique_ptr<ShotPattern>(new SectorPattern());
}

std::unique_ptr<ShotPattern> CreateHomingShotPattern()
{
    return std::unique_ptr<ShotPattern>(new HomingPattern());
}

std::unique_ptr<ShotPattern> CreateShotPatternForKind(CharacterKind kind)
{
    switch (kind)
    {
    case CharacterKind::Player:
        return CreateStraightShotPattern();
    case CharacterKind::Monster:
        return CreateMultiShotPattern();
    case CharacterKind::Summon:
        return CreateHomingShotPattern();
    case CharacterKind::EliteMonster:
        return CreateSectorShotPattern();
    case CharacterKind::Npc:
    case CharacterKind::Object:
    default:
        return CreateDirectionalShotPattern();
    }
}
}
