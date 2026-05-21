#pragma once

#include "Character.h"

#include <memory>

namespace portfolio
{
// Legacy shape:
//   Concrete type creation was mixed into gameplay or registry code.
//
// Refactored shape:
//   CharacterFactory owns concrete type selection and default collaborator setup.
//
//   CharacterKind -> CharacterFactory -> Player/Monster/Summon/Object...
class CharacterFactory
{
public:
    std::unique_ptr<Character> Create(CharacterKind kind) const;
};
}
