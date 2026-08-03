#ifndef WIND_FORCE_GEN_H
#define WIND_FORCE_GEN_H

#include "force_gen.h"
#include "particle.h"
#include <glm/glm.hpp>

namespace melonyx {

    // Applies a constant horizontal acceleration to simulate wind blowing
    // across the level -- distinct from Gravity (constant downward accel)
    // and Drag (velocity-opposing): this is a fixed-direction force
    // independent of the particle's current motion.
    class WindForceGenerator : public ForceGenerator {
    private:
        glm::vec3 windAcceleration;

    public:
        WindForceGenerator(glm::vec3 accel) : windAcceleration(accel) {}

        void SetForce(glm::vec3 accel) { windAcceleration = accel; }

        virtual void UpdateForce(Particle* particle, float duration) override;
    };

} // namespace melonyx

#endif