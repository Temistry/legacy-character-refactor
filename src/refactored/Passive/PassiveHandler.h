#pragma once

#include "../CharacterTypes.h"

#include <string>

namespace portfolio
{
struct PassiveHandler
{
    PassiveTiming timing = PassiveTiming::Passive;
    std::string name;
    int energyDelta = 0;
    int minimumHealth = 0;
    bool oneShot = false;
};

struct PassiveExecutionContext
{
    PassiveTiming timing = PassiveTiming::Passive;
    int currentHealth = 0;
    int frame = 0;
};
}
