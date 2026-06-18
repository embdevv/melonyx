#pragma once
#include "particle.h"

class ParticleContact
{
public:
	float depth;
	melonyx::Particle* particles[2];
	float restitution;
	glm::vec3 contactNormal;
	void Resolve(float time);
	float GetSeparatingSpeed();

protected:
	void ResolveInterpenetration(float time);
	void ResolveVelocity(float time);
};