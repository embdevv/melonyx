/**
 * This link simulates a chain because it only generates a contact (constraint)
 * when the distance between two particles EXCEEDS the max cable length.
 * When the particles are closer than maxLength, the cable is slack — no force
 * is applied, just like a loose chain link. When stretched beyond maxLength,
 * the cable snaps taut and pulls both particles back toward each other.
 * Chaining several of these cables together (particle[0] -> particle[1],
 * particle[1] -> particle[2], etc.) produces a multi-link chain simulation,
 * where each segment can swing freely but cannot overextend.
 */

#include "headers/cable.h"
#include <glm/glm.hpp>

ParticleContact* Cable::GetContact()
{
	float currLen = CurrentLength();

	// Cable is slack — no constraint needed
	if (currLen <= maxLength)
	{
		return nullptr;
	}

	// Cable is taut — pull particles back
	ParticleContact* ret = new ParticleContact();
	ret->particles[0] = particles[0];
	ret->particles[1] = particles[1];

	glm::vec3 dir = particles[1]->Position - particles[0]->Position;
	dir = glm::normalize(dir);

	ret->contactNormal = dir;
	ret->depth = currLen - maxLength;
	ret->restitution = restitution;

	return ret;
}