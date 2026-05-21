#include "refactored/CharacterAccessor.h"
#include "refactored/CharacterSlotStore.h"

#include <cassert>
#include <stdexcept>

using namespace portfolio;

int main()
{
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

