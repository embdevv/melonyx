#include "headers/world_particle.h"
#include <iostream>

namespace melonyx {

	void PhysicsWorld::AddParticle(Particle* toAdd)
	{
		Particles.push_back(toAdd);
		forceRegistry.Add(toAdd, &Gravity);
	}

	void PhysicsWorld::AddContact(melonyx::Particle* p1, melonyx::Particle* p2, float restitution,
		glm::vec3 contactNormal, float depth)
	{
		std::cout << "Overlap detected! depth= " << depth << std::endl;
		ParticleContact* toAdd = new ParticleContact();

		toAdd->particles[0] = p1;
		toAdd->particles[1] = p2;

		toAdd->restitution = restitution;
		toAdd->contactNormal = contactNormal;
		toAdd->depth = depth;

		Contacts.push_back(toAdd);
	}

	void PhysicsWorld::GetOverlaps()
	{
		for (int i = 0; i < Particles.size() - 1; i++)
		{
			std::list<melonyx::Particle*>::iterator a = std::next(Particles.begin(), i);
			
			for (int h = i + 1; h < Particles.size(); h++)
			{
				std::list<melonyx::Particle*>::iterator b = std::next(Particles.begin(), h);

				glm::vec3 mag2Vector = (*a)->Position - (*b)->Position;
				float mag2 = glm::dot(mag2Vector, mag2Vector);

				float rad = (*a)->radius + (*b)->radius;

				float rad2 = rad * rad;


				if (mag2 <= rad2)
				{
					glm::vec3 dir = glm::normalize(mag2Vector);

					float r = rad2 - mag2;
					float depth = sqrt(r);

					float restitution = fmin(
						(*a)->restitution, (*b)->restitution
					);

					AddContact(*a, *b, restitution, dir, depth);
				}
			}
		}
	}

	void PhysicsWorld::GenerateContacts()
	{
		Contacts.clear();

		GetOverlaps();

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

		for (std::list<Cable*>::iterator i = Cables.begin();
			i != Cables.end();
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