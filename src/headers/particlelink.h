#pragma once
#include "particle.h"

class ParticleLink
{
public:
	melonyx::Particle* particles[2];
	virtual ParticleContact* GetContact() { return nullptr;  }

protected:
	float CurrentLength();
};

