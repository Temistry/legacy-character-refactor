#include "refactored/AI/AiDispatcher.h"
#include "refactored/CharacterFactory.h"
#include "refactored/Effects/TimedEffectList.h"
#include "refactored/Passive/PassiveRegistry.h"
#include "refactored/Projectile/ShotPattern.h"
#include "refactored/Skill/SkillExecutor.h"

#include <cassert>
#include <memory>

using namespace portfolio;

int main()
{
    CharacterFactory factory;
    std::unique_ptr<Character> character = factory.Create(CharacterKind::Monster);
    character->Initialize(1, 10);

    // Legacy gap:
    //   Skill cooldown fields, passive flags, timer fields, projectile direction
    //   calculation, and AI branch state were all stored near Character.
    //
    // Refactored shape:
    //   Each module exposes a small API that can be inspected without touching
    //   Character internals.

    PassiveRegistry passives = PassiveRegistry::CreateDefault();
    assert(passives.HandlerCount() == 4);
    assert(passives.HandlerCount(PassiveTiming::Passive) == 1);

    TimedEffectList effects;
    effects.AddEffect("StunLike", 2, true);
    assert(effects.ActiveCount() == 1);
    assert(effects.HasBlockingEffect());
    assert(effects.RemainingFrames("StunLike") == 2);

    std::unique_ptr<ShotPattern> pattern = CreateMultiShotPattern();
    ShotRequest request;
    request.origin.x = 0;
    request.origin.y = 0;
    request.target.x = 3;
    request.target.y = 1;
    request.nearbyEnemyCount = 2;
    ShotPlan plan = pattern->BuildPlan(request);
    assert(plan.spawns.size() == 2);

    AiDispatcher ai;
    UpdateInput input;
    input.nearbyEnemyCount = 2;
    input.requestProjectile = true;
    AiDecision decision = ai.Decide(*character, input);
    assert(decision.intent == AiIntent::RequestShot);

    SkillExecutor skills;
    SkillData skill;
    skill.id = 1;
    skill.category = SkillCategory::Projectile;
    skill.cooldownFrames = 3;
    skill.energyCost = 0;
    SkillRuntimeState state;
    assert(skills.CanExecute(*character, skill, state));
    state = skills.StartCooldown(skill);
    assert(state.remainingCooldown == 3);

    return 0;
}
