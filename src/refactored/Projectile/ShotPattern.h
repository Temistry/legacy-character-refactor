#pragma once

#include "../CharacterTypes.h"

#include <memory>
#include <vector>

namespace portfolio
{
class Character;

struct ShotRequest
{
    Vec2 origin;
    Vec2 target;
    int nearbyEnemyCount = 0;
};

struct ShotSpawn
{
    Vec2 position;
    Vec2 direction;
    int lifeTime = 0;
    bool homing = false;
};

struct ShotPlan
{
    std::vector<ShotSpawn> spawns;
};

// Legacy shape:
//   CreateStraightProjectile/CreateMultiProjectile/etc. repeated coordinate
//   and count calculation inside Character.
//
// Refactored shape:
//   ShotPattern moves each projectile variation behind a strategy interface.
//
//   Character -> ShotPattern::Emit() -> generic projectile event
class ShotPattern
{
public:
    virtual ~ShotPattern() {}
    virtual ShotPlan BuildPlan(const ShotRequest& request) const = 0;
    virtual void Emit(const Character& character, const UpdateInput& input, std::vector<CharacterEvent>& events) const = 0;
};

std::unique_ptr<ShotPattern> CreateStraightShotPattern();
std::unique_ptr<ShotPattern> CreateMultiShotPattern();
std::unique_ptr<ShotPattern> CreateDirectionalShotPattern();
std::unique_ptr<ShotPattern> CreateSectorShotPattern();
std::unique_ptr<ShotPattern> CreateHomingShotPattern();
std::unique_ptr<ShotPattern> CreateShotPatternForKind(CharacterKind kind);
}
