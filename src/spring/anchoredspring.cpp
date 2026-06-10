#include "../headers/anchoredspring.h"

namespace melonyx {

	void AnchoredSpring::UpdateForce(melonyx::Particle* particle, float time)
	{
		glm::vec3 pos = particle->Position;

		glm::vec3 force = pos - anchorPoint;
		
		float mag = glm::length(force);

		float springForce = -springConstant * abs(mag - restLength);

		if (mag > 0)
			force = glm::normalize(force);
		else
			force = glm::vec3(0);

		force = force * springForce;

		particle->AddForce(force);
	}
};