#include "CharacterTuningTable.h"

#include <cstddef>

namespace portfolio
{
namespace
{
const CharacterTuning kFallbackTuning = {
    CharacterKind::Object,
    0,
    0,
    2,
    DefaultShotPattern::Directional
};

const CharacterTuning kCharacterTuningTable[] = {
    { CharacterKind::Player, 1, 0, 2, DefaultShotPattern::Straight },
    { CharacterKind::Npc, 0, 0, 2, DefaultShotPattern::Directional },
    { CharacterKind::Monster, 2, 0, 2, DefaultShotPattern::MultiShot },
    { CharacterKind::Summon, 0, 0, 2, DefaultShotPattern::Homing },
    { CharacterKind::Object, 0, 0, 2, DefaultShotPattern::Directional },
    { CharacterKind::EliteMonster, 3, 1, 2, DefaultShotPattern::Sector }
};
}

const CharacterTuning& FindCharacterTuning(CharacterKind kind)
{
    for (std::size_t i = 0; i < sizeof(kCharacterTuningTable) / sizeof(kCharacterTuningTable[0]); ++i)
    {
        if (kCharacterTuningTable[i].kind == kind)
        {
            return kCharacterTuningTable[i];
        }
    }
    return kFallbackTuning;
}
}
