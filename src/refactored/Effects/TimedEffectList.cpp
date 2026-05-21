#include "TimedEffectList.h"

#include <algorithm>

namespace portfolio
{
void TimedEffectList::Clear()
{
    effects_.clear();
}

void TimedEffectList::AddEffect(int durationFrames)
{
    AddEffect("Effect", durationFrames, false);
}

void TimedEffectList::AddEffect(const std::string& name, int durationFrames, bool blocksAction)
{
    if (durationFrames <= 0)
    {
        return;
    }

    TimedEffect effect;
    effect.name = name;
    effect.remainingFrames = durationFrames;
    effect.blocksAction = blocksAction;
    effects_.push_back(effect);
}

void TimedEffectList::Update(std::vector<CharacterEvent>& events)
{
    int expiredCount = 0;
    for (std::size_t i = 0; i < effects_.size(); ++i)
    {
        --effects_[i].remainingFrames;
        if (effects_[i].remainingFrames <= 0)
        {
            ++expiredCount;
        }
    }

    effects_.erase(
        std::remove_if(effects_.begin(), effects_.end(), [](const TimedEffect& effect) {
            return effect.remainingFrames <= 0;
        }),
        effects_.end());

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
    return static_cast<int>(effects_.size());
}

bool TimedEffectList::HasBlockingEffect() const
{
    for (std::size_t i = 0; i < effects_.size(); ++i)
    {
        if (effects_[i].blocksAction)
        {
            return true;
        }
    }
    return false;
}

int TimedEffectList::RemainingFrames(const std::string& name) const
{
    for (std::size_t i = 0; i < effects_.size(); ++i)
    {
        if (effects_[i].name == name)
        {
            return effects_[i].remainingFrames;
        }
    }
    return 0;
}
}
