#pragma once

// Per-level configuration
// Bottles and obstacles will be populated in later milestones.
struct LevelConfig {
    int   levelNumber;   // 1, 2, 3
    int   bottleQuota;   // bottles needed to advance
    float waterLine;     // y-coord below which pirate drowns
    float windStrength;  // 0 = none, nonzero = Level 3 wind
};

inline LevelConfig getLevelConfig(int level)
{
    switch (level) {
    case 1:  return { 1, 3,  -280.0f, 0.0f };
    case 2:  return { 2, 4,  -280.0f, 0.0f };
    case 3:  return { 3, 6,  -280.0f, 30.0f };
    default: return { 1, 3,  -280.0f, 0.0f };
    }
}