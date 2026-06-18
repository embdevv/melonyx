#include "headers/rod.h"
#include "headers/particlelink.h"

ParticleContact* Rod::GetContact()
{
	float currLen = CurrentLength();

	if (currLen == length)
	{
		return nullptr;
	}
}

ParticleContact* ret = new ParticleContact();
ret->particles[0] = particles[0];
ret->particles[1] = particles[1];
glm::vec3 dir = particles[1]->Position - particles[0]->Position;
dir = dir.Normalize();