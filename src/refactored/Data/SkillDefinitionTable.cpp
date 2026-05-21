#include "SkillDefinitionTable.h"

#include <cstddef>

namespace portfolio
{
namespace
{
const SkillDefinition kSkillDefinitionTable[] = {
    { 1, SkillCategory::Projectile, 3, 0, 1 },
    { 2, SkillCategory::Projectile, 4, 1, 2 },
    { 3, SkillCategory::PassiveTrigger, 5, 1, 0 },
    { 4, SkillCategory::Summon, 6, 2, 0 }
};
}

const SkillDefinition* FindSkillDefinition(int id)
{
    for (std::size_t i = 0; i < sizeof(kSkillDefinitionTable) / sizeof(kSkillDefinitionTable[0]); ++i)
    {
        if (kSkillDefinitionTable[i].id == id)
        {
            return &kSkillDefinitionTable[i];
        }
    }
    return nullptr;
}

SkillData ToSkillData(const SkillDefinition& definition)
{
    SkillData skill;
    skill.id = definition.id;
    skill.category = definition.category;
    skill.cooldownFrames = definition.cooldownFrames;
    skill.energyCost = definition.energyCost;
    skill.toyPower = definition.toyPower;
    return skill;
}
}
