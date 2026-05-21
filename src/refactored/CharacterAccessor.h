#pragma once

#include "Character.h"

#include <cstddef>
#include <vector>

namespace portfolio
{
// Legacy shape:
//   Callers reached characters through raw global array indexes.
//
// Refactored shape:
//   CharacterAccessor keeps array-like syntax while centralizing index and
//   empty-slot validation.
//
//   caller -> CharacterAccessor[index] -> Character&
class CharacterAccessor
{
public:
    explicit CharacterAccessor(std::size_t size);

    void Set(std::size_t index, Character* character);
    void Clear(std::size_t index);
    bool IsValidIndex(std::size_t index) const;
    bool IsOccupied(std::size_t index) const;
    Character* TryGet(std::size_t index);
    const Character* TryGet(std::size_t index) const;
    Character& operator[](std::size_t index);
    const Character& operator[](std::size_t index) const;
    std::size_t Size() const;

private:
    void ValidateIndex(std::size_t index) const;

    std::vector<Character*> slots_;
};
}
