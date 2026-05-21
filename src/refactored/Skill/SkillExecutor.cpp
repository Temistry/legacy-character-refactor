#include "SkillExecutor.h"

#include "../Character.h"

namespace portfolio
{
bool SkillExecutor::CanExecute(const Character& character, const SkillData& skill, const SkillRuntimeState& state) const
{
    return character.IsAlive() &&
           character.Energy() >= skill.energyCost &&
           state.remainingCooldown <= 0;
}

SkillRuntimeState SkillExecutor::StartCooldown(const SkillData& skill) const
{
    SkillRuntimeState state;
    state.remainingCooldown = skill.cooldownFrames;
    state.queued = false;
    return state;
}

void SkillExecutor::Execute(Character&, const SkillData& skill, std::vector<CharacterEvent>& events) const
{
    CharacterEvent event;
    event.name = "SkillExecuted";
    event.value = skill.toyPower;
    events.push_back(event);
}
}
