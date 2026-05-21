#pragma once

#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
// Legacy shape:
//   poison/stun/burn/shield/effect timer fields accumulated in Character.
//
// Refactored shape:
//   TimedEffectList owns duration updates and expiration events.
//
//   AddEffect(duration) -> Update(events) -> EffectExpired
class TimedEffectList
{
public:
    void Clear();
    void AddEffect(int durationFrames);
    void Update(std::vector<CharacterEvent>& events);
    int ActiveCount() const;

private:
    std::vector<int> durations_;
};
}
