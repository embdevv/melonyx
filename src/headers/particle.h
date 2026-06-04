#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace melonyx {

    class Particle
    {
        protected:
            bool isDestroyed = false;
            void UpdatePosition(float dt);
            void UpdateVelocity(float dt);
            
        public:
            glm::vec3 Position;
            glm::vec3 Velocity;
            glm::vec3 Acceleration;

            void Update(float dt);
            void Destroy();
            bool IsDestroyed() {
                return isDestroyed;
            }

            Particle();
    };

}