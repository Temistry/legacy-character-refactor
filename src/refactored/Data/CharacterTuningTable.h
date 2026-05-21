#pragma once

#include "../CharacterTypes.h"

namespace portfolio
{
enum class DefaultShotPattern
{
    Straight,
    MultiShot,
    Directional,
    Sector,
    Homing
};

struct CharacterTuning
{
    CharacterKind kind = CharacterKind::Object;
    int energyGainPerUpdate = 0;
    int projectileBonus = 0;
    int defaultEffectDuration = 0;
    DefaultShotPattern defaultShotPattern = DefaultShotPattern::Directional;
};

// Legacy shape:
//   Type-specific tuning values are selected inside update/projectile branches.
//
// Refactored shape:
//   CharacterTuningTable keeps the toy values in one code-level table first.
//   A later step could move the same rows to CSV, script table, binary table,
//   or editor-authored data asset.
const CharacterTuning& FindCharacterTuning(CharacterKind kind);
}
