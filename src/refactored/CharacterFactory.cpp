#include "CharacterFactory.h"

#include "Passive/PassiveRegistry.h"
#include "Projectile/ShotPattern.h"

namespace portfolio
{
std::unique_ptr<Character> CharacterFactory::Create(CharacterKind kind) const
{
    std::unique_ptr<Character> character;

    switch (kind)
    {
    case CharacterKind::Player:
        character.reset(new PlayerCharacter());
        break;
    case CharacterKind::Npc:
        character.reset(new NpcCharacter());
        break;
    case CharacterKind::Monster:
        character.reset(new MonsterCharacter());
        break;
    case CharacterKind::Summon:
        character.reset(new SummonCharacter());
        break;
    case CharacterKind::Object:
        character.reset(new ObjectCharacter());
        break;
    case CharacterKind::EliteMonster:
        character.reset(new EliteMonsterCharacter());
        break;
    default:
        character.reset(new ObjectCharacter());
        break;
    }

    character->SetPassiveRegistry(PassiveRegistry::CreateDefault());
    character->SetShotPattern(CreateShotPatternForKind(kind));
    return character;
}
}

