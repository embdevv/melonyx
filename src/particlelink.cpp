#include "headers/particlelink.h"

float ParticleLink::CurrentLength()
{
	glm::vec3 ret = particles[0]->Position - particles[1]->Position;
	return ret.Magnitude();
}