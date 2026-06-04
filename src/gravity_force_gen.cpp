#include "headers/gravity_force_gen.h"

void GravityForceGenerator::UpdateForce(melonyx::Particle* particle, float dt)
{
	if (particle->mass <= 0.f) return;
	
	glm::vec3 force = Gravity * particle->mass;
	particle->AddForce(force);
}