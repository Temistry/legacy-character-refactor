#pragma once

#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
class Character;

class ReturnRequestModule
{
public:
    bool TryReturnRequest(Character& character, const UpdateInput& input, std::vector<CharacterEvent>& events) const;
};
}

