#pragma once

#include "../Skill/SkillData.h"

namespace portfolio
{
struct SkillDefinition
{
    int id = 0;
    SkillCategory category = SkillCategory::Utility;
    int cooldownFrames = 0;
    int energyCost = 0;
    int toyPower = 0;
};

// Legacy shape:
//   Skill id checks, cooldown values, and cost values are near execution code.
//
// Refactored shape:
//   SkillDefinitionTable returns a data object, and SkillExecutor consumes
//   SkillData without owning the lookup branch.
const SkillDefinition* FindSkillDefinition(int id);
SkillData ToSkillData(const SkillDefinition& definition);
}
