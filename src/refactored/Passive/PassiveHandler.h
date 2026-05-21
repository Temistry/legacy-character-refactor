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
};
}

