// world_particle.h
#include <list>
#include "particle.h"
#include "force_registry.h"
#include "gravity_force_gen.h"

namespace melonyx {

	class PhysicsWorld {
	public:
		ForceRegistry forceRegistry;
		std::list<melonyx::Particle*> Particles;

		void AddParticle(melonyx::Particle* toAdd);
		void Update(float time);

	private:
		void UpdateParticleList();
		GravityForceGenerator Gravity = GravityForceGenerator(glm::vec3(0, -9.8, 0));
	};
}