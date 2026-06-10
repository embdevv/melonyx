#pragma once
#include "force_gen.h"

namespace melonyx
{
	class AnchoredSpring : public ForceGenerator
	{

	public:
		AnchoredSpring(glm::vec3 pos, float _springConst, float _restLen) :
			anchorPoint(pos), springConstant(_springConst), restLength(_restLen) {}
		
		void UpdateForce(melonyx::Particle* particle, float time) override;


	private:
		glm::vec3 anchorPoint;
		float springConstant;
		float restLength;

	};
};