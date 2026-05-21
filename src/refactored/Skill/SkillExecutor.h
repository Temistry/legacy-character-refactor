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
    bool CanExecute(const Character& character, const SkillData& skill, const SkillRuntimeState& state) const;
    SkillRuntimeState StartCooldown(const SkillData& skill) const;
    void Execute(Character& character, const SkillData& skill, std::vector<CharacterEvent>& events) const;
};
}
