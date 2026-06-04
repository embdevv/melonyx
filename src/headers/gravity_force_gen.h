#pragma once
#include "force_gen.h"

class GravityForceGenerator : public ForceGenerator 
{
private:
	glm::vec3 Gravity = glm::vec3(0, -9.8f, 0);

public:
	GravityForceGenerator(const glm::vec3& gravity) : Gravity(gravity) {}

	void UpdateForce(melonyx::Particle* particle, float dt) override;
};