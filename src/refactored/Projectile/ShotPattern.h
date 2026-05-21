#pragma once

#include "../CharacterTypes.h"

#include <memory>
#include <vector>

namespace portfolio
{
class Character;

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
    virtual void Emit(const Character& character, const UpdateInput& input, std::vector<CharacterEvent>& events) const = 0;
};

std::unique_ptr<ShotPattern> CreateStraightShotPattern();
std::unique_ptr<ShotPattern> CreateMultiShotPattern();
std::unique_ptr<ShotPattern> CreateDirectionalShotPattern();
std::unique_ptr<ShotPattern> CreateSectorShotPattern();
std::unique_ptr<ShotPattern> CreateHomingShotPattern();
std::unique_ptr<ShotPattern> CreateShotPatternForKind(CharacterKind kind);
}
