#include "../headers/force_registry.h"

void ForceRegistry::Add(melonyx::Particle* particle, ForceGenerator* forceGen)
{
	ParticleForceRegistry toAdd;

	toAdd.particle = particle;
	toAdd.forceGen = forceGen;

	registry.push_back(toAdd);
}

void ForceRegistry::Remove(melonyx::Particle* particle, ForceGenerator* forceGen)
{
	registry.remove_if([particle, forceGen](ParticleForceRegistry reg)
		{
			return reg.particle == particle && reg.forceGen == forceGen;
		});
}

void ForceRegistry::Clear()
{
	registry.clear();
}

void ForceRegistry::UpdateForces(float time)
{
	for (std::list<ParticleForceRegistry>::iterator i = registry.begin();
		i != registry.end(); i++)

	{
		i->forceGen->UpdateForce(i->particle, time);
	}
}
