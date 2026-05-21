#include "PassiveRegistry.h"

#include "../Character.h"

namespace portfolio
{
void PassiveRegistry::AddHandler(PassiveTiming timing, const std::string& name, int energyDelta)
{
    PassiveHandler handler;
    handler.timing = timing;
    handler.name = name;
    handler.energyDelta = energyDelta;
    handlers_.push_back(handler);
}

void PassiveRegistry::Execute(PassiveTiming timing, Character& character, std::vector<CharacterEvent>& events) const
{
    for (std::size_t i = 0; i < handlers_.size(); ++i)
    {
        const PassiveHandler& handler = handlers_[i];
        if (handler.timing != timing)
        {
            continue;
        }

        character.AddEnergy(handler.energyDelta);

        CharacterEvent event;
        event.name = "Passive:" + handler.name;
        event.value = character.Energy();
        events.push_back(event);
    }
}

PassiveRegistry PassiveRegistry::CreateDefault()
{
    PassiveRegistry registry;
    registry.AddHandler(PassiveTiming::OneShot, "OneShot", 1);
    registry.AddHandler(PassiveTiming::Passive, "Passive", 1);
    registry.AddHandler(PassiveTiming::PostPassive, "PostPassive", 0);
    registry.AddHandler(PassiveTiming::Dying, "Dying", 0);
    return registry;
}
}

