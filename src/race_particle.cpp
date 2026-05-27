#include "headers/race_particle.h"
#include <iostream>
#include <iomanip>
#include <cmath>

namespace melonyx {

    float RaceParticle::CalculateDistance()
    {
        return glm::length(startPosition);
    }

    RaceParticle::RaceParticle(const std::string& particleName, const glm::vec3& pos,
                               float velocity, float accel)
        : name(particleName), startPosition(pos), initialVelocity(velocity),
          acceleration(accel), timeToCenter(0.0f), finalVelocity(0.0f),
          averageVelocity(0.0f)
    {
        totalDistance = CalculateDistance();
    }

    void RaceParticle::CalculateRaceResults()
    {
        // Using kinematic equation: s = v₀*t + 0.5*a*t²
        // Rearranged as quadratic: 0.5*a*t² + v₀*t - s = 0
        // Solution using quadratic formula: t = (-b ± √(b² - 4ac)) / 2a

        float a = 0.5f * acceleration;
        float b = initialVelocity;
        float c = -totalDistance;

        float discriminant = (b * b) - (4.0f * a * c);

        if (discriminant < 0.0f) {
            timeToCenter = 0.0f;
            return;
        }

        float t1 = (-b + std::sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b - std::sqrt(discriminant)) / (2.0f * a);

        timeToCenter = (t1 > 0.0f) ? t1 : t2;

        // Calculate final velocity: v = v₀ + a*t
        finalVelocity = initialVelocity + (acceleration * timeToCenter);

        // Calculate average velocity: avg_v = (v₀ + v_final) / 2
        averageVelocity = (initialVelocity + finalVelocity) / 2.0f;
    }

    void RaceParticle::DisplayResults()
    {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Name: " << name << std::endl;
        std::cout << "  Time to Center: " << timeToCenter << " seconds" << std::endl;
        std::cout << "  Velocity at Finish: " << finalVelocity << " m/s" << std::endl;
        std::cout << "  Average Velocity: " << averageVelocity << " m/s" << std::endl;
        std::cout << "  Distance: " << totalDistance << " m" << std::endl;
    }

}
