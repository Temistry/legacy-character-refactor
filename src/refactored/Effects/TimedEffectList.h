#pragma once

#include "../CharacterTypes.h"

#include <vector>

namespace portfolio
{
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

