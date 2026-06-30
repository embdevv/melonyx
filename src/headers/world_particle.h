#pragma once
#include <list>
#include "particle.h"
#include "force_registry.h"
#include "gravity_force_gen.h"
#include "particlelink.h"
#include "particlecontact.h"
#include "contactresolver.h"
#include "cable.h"

namespace melonyx {

	class PhysicsWorld {
	public:
		ForceRegistry forceRegistry;
		std::list<melonyx::Particle*> Particles;

		// Contains the list of links (rods, cables, etc.)
		std::list<ParticleLink*> Links;

		// Anchored cables (particle-to-fixed-point constraints)
		std::list<Cable*> Cables;

		void AddParticle(melonyx::Particle* toAdd);
		void Update(float time);
		void AddContact(melonyx::Particle* p1, melonyx::Particle* p2, float restitution,
			glm::vec3 contactNormal, float depth);

	protected:
		// Generates the contacts that need to be resolved
		void GenerateContacts();
		void GetOverlaps();

	private:
		std::list<ParticleContact*> Contacts;
		ContactResolver contactResolver;
		void UpdateParticleList();
		GravityForceGenerator Gravity = GravityForceGenerator(glm::vec3(0, -9.8f, 0));
	};
}