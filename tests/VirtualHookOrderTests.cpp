#include "refactored/Character.h"
#include "refactored/Passive/PassiveRegistry.h"

#include <cassert>
#include <string>
#include <vector>

using namespace portfolio;

namespace
{
// Usage example:
//   The base Character lifecycle controls the ordering.
//
// Legacy problem:
//   Type branches inside one Update method can reorder behavior by accident.
//
// Refactored usage:
//   Derived classes customize narrow hooks while the base class keeps the
//   sequence stable.
class HookOrderCharacter : public Character
{
public:
    HookOrderCharacter()
        : Character(CharacterKind::Object)
    {
    }

protected:
    void OnPreUpdate(const UpdateInput& input) override
    {
        EmitEvent("Hook:PreUpdate", input.frame);
    }

    void OnPostUpdate(const UpdateInput& input) override
    {
        EmitEvent("Hook:PostUpdate", input.frame);
    }
};
}

int main()
{
    HookOrderCharacter character;
    character.SetPassiveRegistry(PassiveRegistry::CreateDefault());
    character.Initialize(1, 10);
    character.ClearEvents();

    UpdateInput input;
    input.frame = 7;
    character.Update(input);

    const std::vector<CharacterEvent>& events = character.Events();
    assert(events.size() == 6);
    assert(events[0].name == "PreUpdate");
    assert(events[1].name == "Hook:PreUpdate");
    assert(events[2].name == "Passive:Passive");
    assert(events[3].name == "Passive:PostPassive");
    assert(events[4].name == "Hook:PostUpdate");
    assert(events[5].name == "Updated");

    return 0;
}
