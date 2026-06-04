#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

namespace melonyx {

    class Particle {
    protected:
        bool isDestroyed = false;
        glm::vec3 accumulatedForce = glm::vec3(0, 0, 0);
        void UpdatePosition(float dt);
        void UpdateVelocity(float dt);

    public:
        glm::vec3 Position;
        glm::vec3 Velocity;
        glm::vec3 Acceleration;

        float damping = 0.999f;
        float mass = 1.0f;

        void Update(float dt);
        void Destroy();
        bool IsDestroyed() { return isDestroyed; }
        void AddForce(glm::vec3 force);
        void ResetForce();

        Particle();
    };

}