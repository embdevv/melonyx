#pragma once
#include "particle.h"
#include "particlecontact.h"

class ParticleLink
{
public:
	melonyx::Particle* particles[2];
	virtual ParticleContact* GetContact() { return nullptr;  }

protected:
	float CurrentLength();
};