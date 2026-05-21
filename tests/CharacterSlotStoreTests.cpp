#include "refactored/CharacterSlotStore.h"

#include <cassert>

using namespace portfolio;

int main()
{
    CharacterSlotStore pool(2);

    Character* first = pool.Acquire(CharacterKind::Player);
    Character* second = pool.Acquire(CharacterKind::Monster);
    Character* third = pool.Acquire(CharacterKind::EliteMonster);

    assert(first != nullptr);
    assert(second != nullptr);
    assert(third == nullptr);
    assert(pool.AllocatedCount() == 2);
    assert(pool.InUseCount() == 2);

    pool.Release(first);
    assert(pool.InUseCount() == 1);

    Character* reused = pool.Acquire(CharacterKind::Player);
    assert(reused == first);
    assert(pool.AllocatedCount() == 2);
    assert(pool.InUseCount() == 2);

    pool.Release(second);
    pool.Release(reused);
    assert(pool.InUseCount() == 0);

    return 0;
}

