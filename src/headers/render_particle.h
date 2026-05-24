// headers/render_particle.h
#pragma once
#include "OpenGLObject.h"
#include "particle.h"
#include <glm/glm.hpp>

class RenderParticle : public melonyx::Particle
{
public:
    melonyx::Particle* PhysicsParticle;
    OpenGLObject*      RenderObject;
    glm::vec3          Color;

    RenderParticle(melonyx::Particle* p, OpenGLObject* obj)
        : PhysicsParticle(p), RenderObject(obj)
    {
        Color = glm::vec3(1.0f, 1.0f, 1.0f);
    }

    RenderParticle(melonyx::Particle* p, OpenGLObject* obj, glm::vec3 c)
        : PhysicsParticle(p), RenderObject(obj), Color(c) {}

    void Draw(GLuint shader, const glm::mat4& projection, const glm::mat4& view);
};