/**
 * @file main.cpp
 * @author Erica Mauriz Barundia
 *
 * Assignment 4 - Chain Simulation
 */

#include <iostream>
#include <string>
#include <chrono>
#include <list>
#include <vector>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headers/main.h"
#include "headers/shader.h"
#include "headers/particle.h"
#include "headers/render_particle.h"
#include "headers/world_particle.h"
#include "headers/cable.h"

using namespace std;
using namespace chrono_literals;

const int    WINDOW_SIZE = 800;
const string WINDOW_TITLE = "Assignment4 Erica Mauriz Barundia";

const int   CHAIN_LINKS = 6;
const float CABLE_LENGTH = 60.0f;
const float PARTICLE_MASS = 10.0f;

int main()
{
    constexpr chrono::nanoseconds timestep(16ms);
    constexpr float timestep_sec = timestep.count() / (float)(1E09);

    if (!glfwInit()) { return -1; }

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_SIZE, WINDOW_SIZE, WINDOW_TITLE.c_str(), NULL, NULL
    );
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress)) { glfwTerminate(); return -1; }

    GLuint shader = compileShaders("shaders/sample.vert", "shaders/sample.frag");
    if (shader == 0) { glfwTerminate(); return -1; }

    ObjMesh sphere;
    if (!sphere.load("3D/sphere.obj")) { glfwTerminate(); return -1; }
    sphere.upload();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        });

    melonyx::PhysicsWorld pWorld;

    // Fixed anchor at the top (not added to world so gravity skips it)
    melonyx::Particle anchor;
    anchor.Position = glm::vec3(0.0f, 250.0f, 0.0f);
    anchor.mass = 1e12f;
    anchor.Velocity = glm::vec3(0.0f);
    anchor.Acceleration = glm::vec3(0.0f);

    // Chain particles
    vector<melonyx::Particle*> chain;
    chain.push_back(&anchor);

    for (int i = 1; i <= CHAIN_LINKS; ++i)
    {
        melonyx::Particle* p = new melonyx::Particle();
        p->Position = glm::vec3(0.0f, 250.0f - i * CABLE_LENGTH, 0.0f);
        p->mass = PARTICLE_MASS;
        p->damping = 0.97f;
        p->Velocity = glm::vec3(0.0f);
        p->Acceleration = glm::vec3(0.0f);
        chain.push_back(p);
        pWorld.AddParticle(p);
    }

    // Nudge last particle to make it swing
    chain.back()->Velocity = glm::vec3(80.0f, 0.0f, 0.0f);

    // Cable links between each pair
    vector<Cable*> cables;
    for (int i = 0; i < (int)chain.size() - 1; ++i)
    {
        Cable* c = new Cable();
        c->particles[0] = chain[i];
        c->particles[1] = chain[i + 1];
        c->maxLength = CABLE_LENGTH;
        c->restitution = 0.0f;
        cables.push_back(c);
        pWorld.Links.push_back(c);
    }

    // Render particles (gold -> blue gradient)
    list<RenderParticle*> renderParticles;
    for (int i = 1; i <= CHAIN_LINKS; ++i)
    {
        float t = (float)(i - 1) / (CHAIN_LINKS - 1);
        glm::vec3 color = glm::mix(glm::vec3(1.0f, 0.8f, 0.1f), glm::vec3(0.1f, 0.3f, 1.0f), t);
        renderParticles.push_back(new RenderParticle(chain[i], &sphere, color));
    }

    // Line VAO for chain segments
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, (CHAIN_LINKS + 1) * 2 * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        curr_time = clock::now();
        auto dur = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
        curr_ns += dur;

        while (curr_ns >= timestep) {
            curr_ns -= timestep;
            pWorld.Update(timestep_sec);

            // Keep anchor fixed every frame
            anchor.Position = glm::vec3(0.0f, 250.0f, 0.0f);
            anchor.Velocity = glm::vec3(0.0f);
            anchor.Acceleration = glm::vec3(0.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        float orthoSize = 350.0f;
        glm::mat4 projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 500.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        for (auto rp : renderParticles)
            rp->Draw(shader, projection, view);

        // Draw chain lines
        {
            vector<float> lineVerts;
            for (int i = 0; i < (int)chain.size() - 1; ++i)
            {
                glm::vec3& a = chain[i]->Position;
                glm::vec3& b = chain[i + 1]->Position;
                lineVerts.push_back(a.x); lineVerts.push_back(a.y); lineVerts.push_back(a.z);
                lineVerts.push_back(b.x); lineVerts.push_back(b.y); lineVerts.push_back(b.z);
            }

            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, lineVerts.size() * sizeof(float), lineVerts.data());

            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3f(glGetUniformLocation(shader, "color"), 0.9f, 0.9f, 0.9f);

            glBindVertexArray(lineVAO);
            glDrawArrays(GL_LINES, 0, (GLsizei)(lineVerts.size() / 3));
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    for (int i = 1; i <= CHAIN_LINKS; ++i) delete chain[i];
    for (auto c : cables) delete c;
    for (auto rp : renderParticles) delete rp;

    return 0;
}