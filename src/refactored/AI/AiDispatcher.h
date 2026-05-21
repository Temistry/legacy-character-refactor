#pragma once

#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
class Character;

enum class AiIntent
{
    None,
    Hold,
    Approach,
    RequestShot
};

struct AiDecision
{
    AiIntent intent = AiIntent::None;
    int priority = 0;
};

class AiDispatcher
{
public:
    AiDecision Decide(const Character& character, const UpdateInput& input) const;
    void Dispatch(Character& character, const UpdateInput& input, std::vector<CharacterEvent>& events) const;
};
}
