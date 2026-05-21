#pragma once

#include <string>

namespace portfolio
{
enum class SkillCategory
{
    Utility,
    Projectile,
    Summon,
    PassiveTrigger
};

struct SkillData
{
    int id = 0;
    SkillCategory category = SkillCategory::Utility;
    std::string debugName;
    int toyPower = 0;
    int cooldownFrames = 0;
    int energyCost = 0;
};

struct SkillRuntimeState
{
    int remainingCooldown = 0;
    bool queued = false;
};
}
