#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include "constants.h"

namespace pirategame {

class Cannon {
public:
    glm::vec2 position;     // where the cannon sits in world space
    float     angleDeg;     // current barrel angle in degrees
    float     power;        // current launch power (world-units/s)

    Cannon(float x, float y)
        : position(x, y),
          angleDeg(45.0f),  // default: 45 degrees
          power(300.0f)     // default: mid power
    {}

    // Rotate barrel — clamped so player can't aim into deck or straight up
    void rotateLeft()  { angleDeg = glm::clamp(angleDeg + ANGLE_STEP, MIN_ANGLE, MAX_ANGLE); }
    void rotateRight() { angleDeg = glm::clamp(angleDeg - ANGLE_STEP, MIN_ANGLE, MAX_ANGLE); }

    // Adjust power
    void powerUp()   { power = glm::clamp(power + POWER_STEP, MIN_POWER, MAX_POWER); }
    void powerDown() { power = glm::clamp(power - POWER_STEP, MIN_POWER, MAX_POWER); }

    // Returns the launch velocity vector based on current angle and power
    glm::vec3 fire() const
    {
        float rad = glm::radians(angleDeg);
        return glm::vec3(
            glm::cos(rad) * power,
            glm::sin(rad) * power,
            0.0f
        );
    }

    // Muzzle tip position (where the pirate spawns from)
    // Offset along the barrel direction by barrelLength units
    glm::vec3 muzzlePosition(float barrelLength = 60.0f) const
    {
        float rad = glm::radians(angleDeg);
        return glm::vec3(
            position.x + glm::cos(rad) * barrelLength,
            position.y + glm::sin(rad) * barrelLength,
            0.0f
        );
    }

    // Power as 0-1 normalized (for drawing the power bar)
    float powerNormalized() const
    {
        return (power - MIN_POWER) / (MAX_POWER - MIN_POWER);
    }
};

} // namespace pirategame