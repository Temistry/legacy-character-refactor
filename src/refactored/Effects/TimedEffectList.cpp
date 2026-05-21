#include "TimedEffectList.h"

#include <algorithm>

namespace portfolio
{
void TimedEffectList::Clear()
{
    durations_.clear();
}

void TimedEffectList::AddEffect(int durationFrames)
{
    if (durationFrames > 0)
    {
        durations_.push_back(durationFrames);
    }
}

void TimedEffectList::Update(std::vector<CharacterEvent>& events)
{
    int expiredCount = 0;
    for (std::size_t i = 0; i < durations_.size(); ++i)
    {
        --durations_[i];
        if (durations_[i] <= 0)
        {
            ++expiredCount;
        }
    }

    durations_.erase(
        std::remove_if(durations_.begin(), durations_.end(), [](int duration) { return duration <= 0; }),
        durations_.end());

    for (int i = 0; i < expiredCount; ++i)
    {
        CharacterEvent event;
        event.name = "EffectExpired";
        event.value = 0;
        events.push_back(event);
    }
}

int TimedEffectList::ActiveCount() const
{
    return static_cast<int>(durations_.size());
}
}

