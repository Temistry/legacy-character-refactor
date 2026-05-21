#pragma once

#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
class Character;

class AiDispatcher
{
public:
    void Dispatch(Character& character, const UpdateInput& input, std::vector<CharacterEvent>& events) const;
};
}

