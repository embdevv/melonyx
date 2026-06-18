/**
 * Cable - A ParticleLink that simulates a chain/cable constraint.
 *
 * A cable only generates a contact when the distance between two particles
 * EXCEEDS the maximum cable length (i.e., it can only pull, never push).
 * This is what makes it behave like a chain link rather than a rod:
 * - A rod enforces EXACT distance (resists both compression AND extension).
 * - A cable/chain only resists extension — the links go slack when close,
 *   but snap taut and pull particles back when stretched too far.
 * When multiple cables are chained together (each linking the next particle
 * to the previous), the result is a simulated chain that hangs, swings, and
 * sags under gravity, exactly like a real chain would.
 */

#pragma once
#include "particlelink.h"
#include "particlecontact.h"

class Cable : public ParticleLink
{
public:
	float maxLength = 1.0f;
	float restitution = 0.0f;

	ParticleContact* GetContact() override;
};