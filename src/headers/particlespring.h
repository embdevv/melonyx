#pragma once
#include "force_gen.h"

class ParticleSpring : public ForceGenerator
{
public:
	ParticleSpring (melonyx::Particle* particle, float _springConst, float _restLen) :
		otherParticle(particle), springConstant(_springConst), restLength(_restLen) {}

	void UpdateForce(melonyx::Particle* particle, float time) override;

private:
	melonyx::Particle* otherParticle;
	float springConstant;
	float restLength;
};