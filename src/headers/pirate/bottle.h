#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "constants.h"

namespace pirategame {

struct Bottle {
    glm::vec2 position;
    float     radius      = BOTTLE_RADIUS;
    bool      collected   = false;

    Bottle(float x, float y) : position(x, y) {}
};

// Given a bottle that was just hit, mark nearby bottles as collected too
// (bowling-pin chain reaction)
inline void checkChainReaction(int hitIndex, std::vector<Bottle>& bottles)
{
    Bottle& hit = bottles[hitIndex];
    for (int i = 0; i < (int)bottles.size(); i++) {
        if (i == hitIndex) continue;
        if (bottles[i].collected) continue;

        float dist = glm::length(bottles[i].position - hit.position);
        if (dist <= CHAIN_RADIUS) {
            bottles[i].collected = true;
            // Recurse so chains can cascade
            checkChainReaction(i, bottles);
        }
    }
}

// Returns how many uncollected bottles remain
inline int bottlesRemaining(const std::vector<Bottle>& bottles)
{
    int count = 0;
    for (const auto& b : bottles)
        if (!b.collected) count++;
    return count;
}

// Returns how many bottles have been collected
inline int bottlesCollected(const std::vector<Bottle>& bottles)
{
    int count = 0;
    for (const auto& b : bottles)
        if (b.collected) count++;
    return count;
}

} // namespace 