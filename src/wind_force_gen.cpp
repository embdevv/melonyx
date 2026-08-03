#include "headers/wind_force_gen.h"
#include "headers/particle.h"

namespace melonyx {

    void WindForceGenerator::UpdateForce(Particle* particle, float duration) {
        if (!particle) return;

        // Multiply by mass so windAcceleration behaves like GRAVITY does --
        // an acceleration value, not a raw force -- keeping units consistent
        // for level tuning (heavier particles aren't blown around less).
        particle->AddForce(windAcceleration * particle->mass);
    }

} // namespace melonyx