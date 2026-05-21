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
    Character& operator[](std::size_t index);
    const Character& operator[](std::size_t index) const;
    std::size_t Size() const;

private:
    void ValidateIndex(std::size_t index) const;

    std::vector<Character*> slots_;
};
}
