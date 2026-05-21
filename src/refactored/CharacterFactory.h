#pragma once

#include "Character.h"

#include <memory>

namespace portfolio
{
class CharacterFactory
{
public:
    std::unique_ptr<Character> Create(CharacterKind kind) const;
};
}

