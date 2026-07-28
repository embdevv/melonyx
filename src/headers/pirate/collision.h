#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "particle.h"
#include "bottle.h"
#include "constants.h"

namespace pirategame {

struct CollisionResult {
    bool hitBottle    = false;
    bool hitWater     = false;
    int  bottlesHit   = 0;
};

// Check pirate particle against all bottles and water line.
// Modifies bottles in-place (marks collected).
// Returns what happened this frame.
inline CollisionResult checkCollisions(
    melonyx::Particle&   pirate,
    std::vector<Bottle>& bottles,
    float                waterLine)
{
    CollisionResult result;

    // ── Pirate vs. water ─────────────────────────────────────────────────
    if (pirate.Position.y <= waterLine) {
        result.hitWater = true;
        return result;  // no point checking bottles if drowned
    }

    // ── Pirate vs. bottles (circle-circle) ───────────────────────────────
    for (int i = 0; i < (int)bottles.size(); i++) {
        if (bottles[i].collected) continue;

        glm::vec2 piratePos2D(pirate.Position.x, pirate.Position.y);
        float dist = glm::length(piratePos2D - bottles[i].position);

        if (dist < pirate.radius + bottles[i].radius) {
            bottles[i].collected = true;
            result.hitBottle     = true;
            result.bottlesHit++;

            // Trigger chain reaction for nearby bottles
            checkChainReaction(i, bottles);

            // Count any additional bottles knocked by chain
            result.bottlesHit = bottlesCollected(bottles);
        }
    }

    return result;
}

} // namespace 