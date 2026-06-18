#include "headers/particlelink.h"
#include <glm/glm.hpp>

float ParticleLink::CurrentLength()
{
	glm::vec3 ret = particles[0]->Position - particles[1]->Position;
	return glm::length(ret);
}