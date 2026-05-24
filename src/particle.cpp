#include "headers/particle.h"

namespace melonyx {
    void Particle::UpdatePosition(float dt)
    {
        this->Position += (this->Velocity * dt) + 
        ((1.0f/2.0f) * (this->Acceleration * dt * dt));
    }

    void Particle::UpdateVelocity(float dt)
    {
        this->Velocity += this->Acceleration * dt;
    }

    void Particle::Update(float time)
    {
        this->Position += this->Velocity * time;
        this->Velocity += this->Acceleration * time;
    }

    Particle::Particle()
    {
        this->Position = glm::vec3(0, 0, 0);
        this->Velocity = glm::vec3(0, 0, 0);
        this->Acceleration = glm::vec3(0, 0, 0);
    }

}