#pragma once

#include "SkillData.h"
#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
class Character;

class SkillExecutor
{
public:
    void Execute(Character& character, const SkillData& skill, std::vector<CharacterEvent>& events) const;
};
}

