#include "headers/particle.h"
#include "headers/particlecontact.h"

float ParticleContact::GetSeparatingSpeed()
{
	glm::vec3 relativeVelocity = particles[0]->Velocity;
	if (particles[1]) relativeVelocity -= particles[1]->Velocity;
	return glm::dot(relativeVelocity, contactNormal);
}

void ParticleContact::ResolveVelocity(float time)
{
	float separatingSpeed = GetSeparatingSpeed();

	if (separatingSpeed > 0) return;

	float newSepSpeed = -separatingSpeed * restitution;
	float deltaSpeed = newSepSpeed - separatingSpeed;

	float totalInvMass = 1.0f / particles[0]->mass;
	if (particles[1]) totalInvMass += 1.0f / particles[1]->mass;

	if (totalInvMass <= 0) return;

	float impulse = deltaSpeed / totalInvMass;
	glm::vec3 impulsePerIMass = contactNormal * impulse;

	particles[0]->Velocity += impulsePerIMass * (1.0f / particles[0]->mass);
	if (particles[1])
		particles[1]->Velocity += impulsePerIMass * -(1.0f / particles[1]->mass);
}

void ParticleContact::ResolveInterpenetration(float time)
{
	if (depth <= 0) return;

	float totalInvMass = 1.0f / particles[0]->mass;
	if (particles[1]) totalInvMass += 1.0f / particles[1]->mass;

	if (totalInvMass <= 0) return;

	float totalMoveByMass = depth / totalInvMass;
	glm::vec3 moveByMass = contactNormal * totalMoveByMass;

	particles[0]->Position += moveByMass * (1.0f / particles[0]->mass);
	if (particles[1])
		particles[1]->Position += moveByMass * -(1.0f / particles[1]->mass);

	depth = 0;
}

void ParticleContact::Resolve(float time)
{
	ResolveVelocity(time);
	ResolveInterpenetration(time);
}