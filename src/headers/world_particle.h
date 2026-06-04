// world_particle.h
#include <list>
#include "particle.h"

namespace melonyx {

	class PhysicsWorld {
	public:
		std::list<melonyx::Particle*> Particles;

		void AddParticle(melonyx::Particle* toAdd);
		void Update(float time);

	private:
		void UpdateParticleList();
	};
}