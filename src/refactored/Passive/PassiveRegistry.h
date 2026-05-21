#pragma once

#include "PassiveHandler.h"

#include <vector>

namespace portfolio
{
class Character;

class PassiveRegistry
{
public:
    void AddHandler(PassiveTiming timing, const std::string& name, int energyDelta);
    void Execute(PassiveTiming timing, Character& character, std::vector<CharacterEvent>& events) const;

    static PassiveRegistry CreateDefault();

private:
    std::vector<PassiveHandler> handlers_;
};
}

