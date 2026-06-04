#pragma once
#include "particle.h"

class ForceGenerator
{
public:
	virtual void UpdateForce(melonyx::Particle* particle, float dt)
	{
		particle->AddForce(glm::vec3(0, 0, 0));
	}
};