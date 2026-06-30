#pragma once
#include "particlelink.h"
#include "particlecontact.h"

namespace melonyx
{

	class Rod : public ParticleLink
	{
	public:
		float length = 1;
		float restitution = 0;

		ParticleContact* GetContact() override;
	};
}