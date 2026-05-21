#include "AiDispatcher.h"

#include "../Character.h"

namespace portfolio
{
void AiDispatcher::Dispatch(Character&, const UpdateInput&, std::vector<CharacterEvent>&) const
{
    // The toy dispatcher is intentionally quiet so behavior equivalence tests
    // focus on structure and ordering instead of domain-specific AI rules.
}
}

