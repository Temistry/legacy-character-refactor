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
    void Emit(const Character&, const UpdateInput&, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(1, events);
    }
};

class MultiShotPattern : public ShotPattern
{
public:
    void Emit(const Character&, const UpdateInput& input, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(CharacterMath::ProjectileCountForKind(CharacterKind::Monster, input.nearbyEnemyCount), events);
    }
};

class DirectionalPattern : public ShotPattern
{
public:
    void Emit(const Character&, const UpdateInput&, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(1, events);
    }
};

class SectorPattern : public ShotPattern
{
public:
    void Emit(const Character&, const UpdateInput& input, std::vector<CharacterEvent>& events) const override
    {
        EmitProjectileEvent(CharacterMath::ProjectileCountForKind(CharacterKind::EliteMonster, input.nearbyEnemyCount), events);
    }
};

class HomingPattern : public ShotPattern
{
public:
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

