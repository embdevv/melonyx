#pragma once

#include <glm/glm.hpp>
#include <string>
#include <cmath>

namespace melonyx {

    class RaceParticle
    {
    protected:
        glm::vec3 startPosition;
        float initialVelocity;
        float acceleration;
        float totalDistance;

        float CalculateDistance();

    public:
        std::string name;
        float timeToCenter;
        float finalVelocity;
        float averageVelocity;

        RaceParticle(const std::string& particleName, const glm::vec3& pos, 
                     float velocity, float accel);
        
        void CalculateRaceResults();
        void DisplayResults();
    };

}
