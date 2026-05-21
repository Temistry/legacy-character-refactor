#pragma once

#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
class Character;

// Legacy shape:
//   Return-request checks were another branch inside the main update path.
//
// Refactored shape:
//   ReturnRequestModule owns the return-request decision and event emission.
class ReturnRequestModule
{
public:
    bool TryReturnRequest(Character& character, const UpdateInput& input, std::vector<CharacterEvent>& events) const;
};
}
