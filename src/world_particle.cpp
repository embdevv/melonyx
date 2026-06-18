#include "headers/world_particle.h"

namespace melonyx {

	void PhysicsWorld::AddParticle(Particle* toAdd)
	{
		Particles.push_back(toAdd);
		forceRegistry.Add(toAdd, &Gravity);
	}

	void PhysicsWorld::GenerateContacts()
	{
		Contacts.clear();

		for (std::list<ParticleLink*>::iterator i = Links.begin();
			i != Links.end();
			i++)
		{
			ParticleContact* contact = (*i)->GetContact();

			if (contact != nullptr)
			{
				Contacts.push_back(contact);
			}
		}
	}

	void PhysicsWorld::Update(float time)
	{
		UpdateParticleList();

		forceRegistry.UpdateForces(time);

		for (std::list<Particle*>::iterator p = Particles.begin();
			p != Particles.end();
			p++)
		{
			(*p)->Update(time);
		}

		GenerateContacts();

		if (Contacts.size() > 0)
		{
			contactResolver.ResolveContacts(Contacts, time);
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