#pragma once

#include "../CharacterTypes.h"

#include <string>
#include <vector>

namespace portfolio
{
struct TimedEffect
{
    std::string name;
    int remainingFrames = 0;
    bool blocksAction = false;
};

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
    void AddEffect(const std::string& name, int durationFrames, bool blocksAction);
    void Update(std::vector<CharacterEvent>& events);
    int ActiveCount() const;
    bool HasBlockingEffect() const;
    int RemainingFrames(const std::string& name) const;

private:
    std::vector<TimedEffect> effects_;
};
}
