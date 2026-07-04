// render_particle.cpp
#include "headers/render_particle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void RenderParticle::Draw(GLuint shader, const glm::mat4& projection, const glm::mat4& view)
{
    if (PhysicsParticle->IsDestroyed()) return;

    // Translate to particle position and scale to match its physics radius
    // (was hardcoded to 20.0f before; now reflects the actual particle->radius
    // so the "Particle Radius" demo parameter visually matches the physics/collision size)
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), PhysicsParticle->Position);
    transform = glm::scale(transform, glm::vec3(PhysicsParticle->radius));

    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"),
        1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"),
        1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "transform"),
        1, GL_FALSE, glm::value_ptr(transform));
    glUniform3fv(glGetUniformLocation(shader, "color"),
        1, glm::value_ptr(Color));

    RenderObject->draw();

}