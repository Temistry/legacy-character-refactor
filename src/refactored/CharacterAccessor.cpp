#include "CharacterAccessor.h"

#include <stdexcept>

namespace portfolio
{
CharacterAccessor::CharacterAccessor(std::size_t size)
    : slots_(size, nullptr)
{
}

void CharacterAccessor::Set(std::size_t index, Character* character)
{
    ValidateIndex(index);
    slots_[index] = character;
}

Character& CharacterAccessor::operator[](std::size_t index)
{
    ValidateIndex(index);
    if (slots_[index] == nullptr)
    {
        throw std::runtime_error("character slot is empty");
    }
    return *slots_[index];
}

const Character& CharacterAccessor::operator[](std::size_t index) const
{
    ValidateIndex(index);
    if (slots_[index] == nullptr)
    {
        throw std::runtime_error("character slot is empty");
    }
    return *slots_[index];
}

std::size_t CharacterAccessor::Size() const
{
    return slots_.size();
}

void CharacterAccessor::ValidateIndex(std::size_t index) const
{
    if (index >= slots_.size())
    {
        throw std::out_of_range("character index is out of range");
    }
}
}

