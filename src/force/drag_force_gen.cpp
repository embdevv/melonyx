#include "../headers/drag_force_gen.h"

void DragForceGenerator::UpdateForce(melonyx::Particle* particle, float time)
{
	glm::vec3 force = glm::vec3(0, 0, 0);
	glm::vec3 curr_vel = particle->Velocity;

	float mag = glm::length(curr_vel);

	if (mag <= 0) return;

	float dragF = (k1 * mag) + (k2 * mag);

	glm::vec3 dir = glm::normalize(curr_vel);

	particle->AddForce(dir * -dragF);
}