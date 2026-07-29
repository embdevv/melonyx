#pragma once

#include "../particle.h"
#include "constants.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace pirategame {

    class Pirate : public melonyx::Particle {
    public:
        float angularVelocity = 0.0f;  // degrees per second
        float rotation = 0.0f;  // current rotation in degrees
        bool  isLaunched = false;
        bool  inWater = false;
        bool  landed = false;

        // Call this at launch — sets spin based on power
        void onLaunch(float power)
        {
            isLaunched = true;
            inWater = false;
            landed = false;
            rotation = 0.0f;
            angularVelocity = power * SPIN_FACTOR;
        }

        // Call this when pirate hits an obstacle or bottle
        void onCollisionHit(float impulseMagnitude)
        {
            angularVelocity += impulseMagnitude * SPIN_MULTIPLIER;
        }

        // Call every physics step while launched
        void updateRotation(float dt)
        {
            if (!isLaunched) return;
            rotation += angularVelocity * dt;

            // Bleed off spin gradually
            angularVelocity *= 0.995f;
        }

        // Returns model matrix with position + Z rotation applied
        glm::mat4 modelMatrix(float scale = 20.0f) const
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, Position);
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(scale));
            return model;
        }

        void reset(glm::vec3 spawnPos)
        {
            Position = spawnPos;
            Velocity = glm::vec3(0.0f);
            Acceleration = glm::vec3(0.0f);
            mass = 1.0f;
            damping = 0.999f;
            radius = PIRATE_RADIUS;
            angularVelocity = 0.0f;
            rotation = 0.0f;
            isLaunched = false;
            inWater = false;
            landed = false;
            ResetForce();
        }
    };

} // namespace pirategame