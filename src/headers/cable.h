#pragma once
#include "particle.h"
#include "particlecontact.h"

namespace melonyx
{
    class Cable
    {
    public:
        Cable(glm::vec3 anchor, float maxLen, float _restitution)
            : anchorPoint(anchor), maxLength(maxLen), restitution(_restitution) {
        }

        melonyx::Particle* particle = nullptr;
        
        // Check current distance from the particle to the anchor
        ParticleContact* GetContact(); 

    private:
        glm::vec3 anchorPoint;
        float maxLength;
        float restitution;
    };
}