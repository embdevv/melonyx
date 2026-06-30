#include "headers/cable.h"
#include <glm/glm.hpp>

namespace melonyx
{
    ParticleContact* Cable::GetContact()
    {
        glm::vec3 toParticle = particle->Position - anchorPoint;
        float currLen = glm::length(toParticle);

        // Cable is slack, no constraint needed
        if (currLen <= maxLength)
        {
            return nullptr;
        }

        ParticleContact* ret = new ParticleContact();
        ret->particles[0] = particle;
        ret->particles[1] = nullptr; // anchor is fixed, not a real particle

        glm::vec3 dir = glm::normalize(toParticle);

        ret->contactNormal = dir * -1.0f; // pull particle back toward anchor
        ret->depth = currLen - maxLength;
        ret->restitution = restitution;

        return ret;
    }
}