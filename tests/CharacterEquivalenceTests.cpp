#include "legacy/LegacyCharacter.h"
#include "refactored/CharacterFactory.h"

#include <cassert>
#include <memory>
#include <vector>

using namespace portfolio;

namespace
{
void RunScenario(CharacterKind kind)
{
    LegacyCharacter legacy;
    legacy.Initialize(10, kind, 10);

    CharacterFactory factory;
    std::unique_ptr<Character> refactored = factory.Create(kind);
    refactored->Initialize(10, 10);

    UpdateInput firstUpdate;
    firstUpdate.frame = 1;
    firstUpdate.nearbyEnemyCount = 2;
    firstUpdate.requestProjectile = true;
    firstUpdate.requestReturn = true;

    legacy.Update(firstUpdate);
    refactored->Update(firstUpdate);

    legacy.ApplyDamage(4);
    refactored->ApplyDamage(4);

    UpdateInput secondUpdate;
    secondUpdate.frame = 2;
    secondUpdate.nearbyEnemyCount = 1;
    secondUpdate.requestProjectile = false;
    secondUpdate.requestReturn = false;

    legacy.Update(secondUpdate);
    refactored->Update(secondUpdate);

    legacy.ApplyDamage(20);
    refactored->ApplyDamage(20);

    assert(legacy.Snapshot() == refactored->Snapshot());
    assert(legacy.Events() == refactored->Events());
}
}

int main()
{
    RunScenario(CharacterKind::Player);
    RunScenario(CharacterKind::Npc);
    RunScenario(CharacterKind::Monster);
    RunScenario(CharacterKind::Summon);
    RunScenario(CharacterKind::Object);
    RunScenario(CharacterKind::EliteMonster);
    return 0;
}

