#include "SkillExecutor.h"

#include "../Character.h"

namespace portfolio
{
void SkillExecutor::Execute(Character&, const SkillData& skill, std::vector<CharacterEvent>& events) const
{
    CharacterEvent event;
    event.name = "SkillExecuted";
    event.value = skill.toyPower;
    events.push_back(event);
}
}

