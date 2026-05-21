#include "ReturnRequestModule.h"

#include "../Character.h"

namespace portfolio
{
bool ReturnRequestModule::TryReturnRequest(Character&, const UpdateInput& input, std::vector<CharacterEvent>& events) const
{
    if (!input.requestReturn)
    {
        return false;
    }

    CharacterEvent event;
    event.name = "ReturnRequestChecked";
    event.value = 1;
    events.push_back(event);
    return true;
}
}

