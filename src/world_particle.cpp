// world_particle.cpp
#include "headers/world_particle.h"

namespace melonyx {
	void PhysicsWorld::AddParticle(Particle* toAdd)
	{
		Particles.push_back(toAdd);
	}

	void PhysicsWorld::Update(float time)
	{
		UpdateParticleList();

		for (std::list<Particle*>::iterator p = Particles.begin();
			p != Particles.end();
			p++) 
		{
			(*p)->Update(time);
		}
	}

	void PhysicsWorld::UpdateParticleList()
	{
		Particles.remove_if(
			[](Particle* p) {
				return p->IsDestroyed();			
			}
		);
	}

}