#pragma once

#include "CharacterFactory.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace portfolio
{
// Legacy shape:
//   Allocation and reuse policy appeared at multiple call sites.
//
// Refactored shape:
//   CharacterSlotStore owns acquire/release state for reusable Character slots.
//
//   Acquire(kind) -> in-use slot
//   Release(ptr)  -> reusable slot
class CharacterSlotStore
{
public:
    explicit CharacterSlotStore(std::size_t capacity);

    Character* Acquire(CharacterKind kind);
    void Release(Character* character);
    bool Contains(const Character* character) const;
    std::size_t AvailableCount() const;

    std::size_t Capacity() const;
    std::size_t AllocatedCount() const;
    std::size_t InUseCount() const;

private:
    struct Slot
    {
        std::unique_ptr<Character> object;
        bool inUse = false;
    };

    std::size_t capacity_;
    CharacterFactory factory_;
    std::vector<Slot> slots_;
};
}
