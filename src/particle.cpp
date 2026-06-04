#include "headers/particle.h"

namespace melonyx {
    void Particle::UpdatePosition(float dt)
    {
        this->Position += (this->Velocity * dt) + 
        ((1.0f/2.0f) * (this->Acceleration * dt * dt));
    }

    void Particle::UpdateVelocity(float dt)
    {
		float d_mass = glm::max(std::numeric_limits<float>::min(), mass);
		this->Acceleration += accumulatedForce * (1 / d_mass);
        this->Velocity += this->Acceleration * dt;
        this->Velocity = this->Velocity * powf(damping, dt);
    }

    void Particle::Update(float time)
    {
		if (this->isDestroyed) return;
        this->UpdatePosition(time);
        this->UpdateVelocity(time);
        this->ResetForce();
    }

    Particle::Particle()
    {
        this->Position = glm::vec3(0, 0, 0);
        this->Velocity = glm::vec3(0, 0, 0);
        this->Acceleration = glm::vec3(0, 0, 0);
    }

    void Particle::Destroy()
    {
        this->isDestroyed = true;
    }

    void Particle::AddForce(glm::vec3 force)
    {
        this->accumulatedForce += force;

    }

    void Particle::ResetForce()
    {
        float d_mass = glm::max(std::numeric_limits<float>::min(), mass);
		this->Acceleration -= accumulatedForce * (1 / d_mass);
        this->accumulatedForce = glm::vec3(0, 0, 0);
    }
}