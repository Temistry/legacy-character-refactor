#include "refactored/CharacterAccessor.h"
#include "refactored/CharacterSlotStore.h"

#include <cassert>
#include <stdexcept>

using namespace portfolio;

int main()
{
    // Legacy problem:
    //   Raw global array access makes invalid indexes and empty slots easy to
    //   miss at call sites.
    //
    // Refactored usage:
    //   CharacterAccessor keeps array-like syntax but validates the boundary.
    CharacterSlotStore pool(1);
    Character* character = pool.Acquire(CharacterKind::Player);
    assert(character != nullptr);
    character->Initialize(3, 10);

    CharacterAccessor accessor(2);
    accessor.Set(0, character);
    assert(accessor[0].Id() == 3);

    bool outOfRangeThrown = false;
    try
    {
        accessor[2].Id();
    }
    catch (const std::out_of_range&)
    {
        outOfRangeThrown = true;
    }
    assert(outOfRangeThrown);

    bool emptySlotThrown = false;
    try
    {
        accessor[1].Id();
    }
    catch (const std::runtime_error&)
    {
        emptySlotThrown = true;
    }
    assert(emptySlotThrown);

    return 0;
}
