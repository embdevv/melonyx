#include "headers/drag_force_gen.h"
#include "headers/particle.h"
#include <glm/glm.hpp>

namespace melonyx {

    void DragForceGenerator::UpdateForce(Particle* particle, float duration) {
        if (!particle) return;

        glm::vec3 velocity = particle->Velocity;
        float speed = glm::length(velocity);

        // Avoid division by zero or normalizing tiny vectors near zero
        if (speed < 0.0001f) return;

        // Calculate total drag force magnitude: F_drag = k1 * v + k2 * v^2
        float dragMagnitude = (k1 * speed) + (k2 * speed * speed);

        // Compute force vector opposite to velocity direction
        glm::vec3 dragForce = -glm::normalize(velocity) * dragMagnitude;

        particle->AddForce(dragForce);
    }

} // namespace melonyx