#pragma once

#include <list>
#include "particle.h"
#include "force_gen.h"

class ForceRegistry
{
protected:
	struct ParticleForceRegistry
	{
		melonyx::Particle* particle;
		ForceGenerator* forceGen;
	};

	std::list<ParticleForceRegistry> registry;

public:
	void Add(melonyx::Particle* particle, ForceGenerator* forceGen);
	void Remove(melonyx::Particle* particle, ForceGenerator* forceGen);
	void Clear();
	void UpdateForces(float dt);

};