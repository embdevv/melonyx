#include "headers/particle.h"
#include "headers/particlecontact.h"

void ParticleContact::ResolveInterpenetration(float time)
{
	if (depth <= 0) return;

	float totalMass = (float)1 / particles[0]->mass;
	if (particles[1]) totalMass += (float)1 / particles[1]->mass;

	if (totalMass <= 0) return;

	float totalMoveByMass = depth / totalMass;

	glm::vec3 moveByMass = contactNormal * totalMoveByMass;
	
	glm::vec3 P_a = moveByMass * ((float)1 / particles[0]->mass);

	particles[0]->Position += P_a;

	if (particles[1])
	{
		glm::vec3 P_b = moveByMass * (-(float)1 / particles[1]->mass);
		particles[1]->Position += P_b;
	}

	depth = 0;
}

void ParticleContact::Resolve(float time)
{
	ResolveVelocity(time);
	ResolveInterpenetration(time);
}