#include "AiDispatcher.h"

#include "../Character.h"

namespace portfolio
{
AiDecision AiDispatcher::Decide(const Character& character, const UpdateInput& input) const
{
    AiDecision decision;
    if (!character.IsAlive())
    {
        decision.intent = AiIntent::None;
    }
    else if (input.nearbyEnemyCount > 0)
    {
        decision.intent = input.requestProjectile ? AiIntent::RequestShot : AiIntent::Approach;
        decision.priority = input.nearbyEnemyCount;
    }
    else
    {
        decision.intent = AiIntent::Hold;
    }
    return decision;
}

void AiDispatcher::Dispatch(Character& character, const UpdateInput& input, std::vector<CharacterEvent>&) const
{
    const AiDecision decision = Decide(character, input);
    (void)decision;

    // The toy dispatcher is intentionally quiet so behavior equivalence tests
    // focus on structure and ordering instead of domain-specific AI rules.
}
}
