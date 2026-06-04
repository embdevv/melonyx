/**
 * @file melonyx.cpp
 * @author Erica Mauriz Barundia
 *
 * Assignment:
 * - Square window with name in title
 * - Orthographic camera
 * - Dark red sphere rendered at center
 */

#include <iostream>
#include <string>
#include <chrono>
#include <list>

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

#include "headers/force_gen.h"
#include "headers/force_registry.h"
#include "headers/gravity_force_gen.h"
#include "headers/drag_force_gen.h"

#include <cstdlib>

using namespace std;
using namespace chrono_literals;

// ===== WINDOW =====
const int    WINDOW_SIZE = 800;
const string WINDOW_TITLE = "Erica Mauriz Barundia";

bool AtCenter(const melonyx::Particle& p)
{
    return p.Position.x >= 0.0f;
}

// ===== MAIN =====
int main()
{
    constexpr chrono::nanoseconds timestep(16ms);
    constexpr float timestep_sec = timestep.count() / (float)(1E09);

    if (!glfwInit()) {
        cerr << "GLFW init failed" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_SIZE, WINDOW_SIZE,
        WINDOW_TITLE.c_str(),
        NULL, NULL
    );

    if (!window) {
        cerr << "Window creation failed" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL(glfwGetProcAddress)) {
        cerr << "Glad init failed" << endl;
        return -1;
    }

    cout << "OpenGL: " << glGetString(GL_VERSION) << endl;
    cout << "Melonyx Engine initialized" << endl;

    // Compile shaders
    GLuint shader = compileShaders("shaders/sample.vert", "shaders/sample.frag");
    if (shader == 0) {
        cerr << "Shader compilation failed" << endl;
        glfwTerminate();
        return -1;
    }
    cout << "Shaders compiled successfully" << endl;

    // Build sphere
    ObjMesh sphere;
    if (!sphere.load("3D/sphere.obj"))
    {
        cerr << "Failed to load sphere.obj" << endl;
        glfwTerminate();
        return -1;
    }
    sphere.upload();
    sphere.color = glm::vec3(0.8f, 0.1f, 0.1f);

    // OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);

    // ESC to close
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        });

    // ===== PHYSICS WORLD SETUP =====
    
    melonyx::PhysicsWorld pWorld = melonyx::PhysicsWorld();

    glm::vec3 velocity = glm::vec3(100.0f, 0.0f, 0.0f);
    glm::vec3 accel = glm::vec3(0.0f, 0.0f, 0.0f);

    
    // Create particle and add to world
    melonyx::Particle p1;
    p1.Position = glm::vec3(-400, 200, 0);
    p1.mass = 1;
    p1.damping = 0.9f;
    p1.Velocity = velocity;
    p1.Acceleration = accel;
    pWorld.AddParticle(&p1);
   

    DragForceGenerator drag = DragForceGenerator(0.14, 0.1);
    pWorld.forceRegistry.Add(&p1, &drag);

    melonyx::Particle p2;
    p2.Position = glm::vec3(-400, 0, 0);
    p2.Velocity = velocity;
    p2.mass = 1;
    p2.damping = 0.9f;
    p2.Acceleration = accel;
    pWorld.AddParticle(&p2);

    melonyx::Particle p3;
    p3.Position = glm::vec3(-400, -200, 0);
    p3.Velocity = velocity;
    p3.Acceleration = accel;
    pWorld.AddParticle(&p3);

    // Create render particle linked to p1 and sphere
    RenderParticle rp1(&p1, &sphere, glm::vec3(1.0f, 0.0f, 0.0f)); // red
	RenderParticle rp2(&p2, &sphere, glm::vec3(1.0f, 1.0f, 0.0f)); // yellow
    RenderParticle rp3(&p3, &sphere, glm::vec3(0.0f, 1.0f, 0.0f)); // green
    
    std::list<RenderParticle*> RenderParticles;
    RenderParticles.push_back(&rp1);
    RenderParticles.push_back(&rp2);
    RenderParticles.push_back(&rp3);

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    // ===== RENDER LOOP =====
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // --- Frame timing ---
        curr_time = clock::now();
        auto dur = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
        curr_ns += dur;

        // --- Physics update ---
        while (curr_ns >= timestep) {
            curr_ns -= timestep;

            // Update all particles via physics world
            //pWorld.Update(timestep_sec);

            //if (AtCenter(p1)) p1.Destroy();
            //if (AtCenter(p2)) p2.Destroy();
            //if (AtCenter(p3)) p3.Destroy();

            //p1.AddForce(glm::vec3(6000, 0, 0));

            //cout << "Melonyx Update" << endl;
            pWorld.Update(timestep_sec);

            // Bounce off window bounds
            const float boundary = 425.0f;
            if (p1.Position.x >= boundary || p1.Position.x <= -boundary) {
                p1.Position.x = glm::clamp(p1.Position.x, -boundary, boundary);
                p1.Velocity.x *= -1.0f;
            }

            if (p1.Position.y >= boundary || p1.Position.y <= -boundary) {
                p1.Position.y = glm::clamp(p1.Position.y, -boundary, boundary);
                p1.Velocity.y *= -1.0f;
            }

            if (p2.Position.x >= boundary || p2.Position.x <= -boundary) {
                p2.Position.x = glm::clamp(p2.Position.x, -boundary, boundary);
                p2.Velocity.x *= -1.0f;
            }

            if (p2.Position.y >= boundary || p2.Position.y <= -boundary) {
                p2.Position.y = glm::clamp(p2.Position.y, -boundary, boundary);
                p2.Velocity.y *= -1.0f;
            }

            if (p3.Position.x >= boundary || p3.Position.x <= -boundary) {
                p3.Position.x = glm::clamp(p3.Position.x, -boundary, boundary);
                p3.Velocity.x *= -1.0f;
            }

            if (p3.Position.y >= boundary || p3.Position.y <= -boundary) {
                p3.Position.y = glm::clamp(p3.Position.y, -boundary, boundary);
                p3.Velocity.y *= -1.0f;
            }

            //cout << "Position: " << p1.Position.x << ", "
            //    << p1.Position.y << ", " << p1.Position.z << endl;
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        float orthoSize = 450.0f;
        glm::mat4 projection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize, orthoSize,
            0.1f, 1000.0f
        );

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 500.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        for (auto it = RenderParticles.begin(); it != RenderParticles.end(); it++) {
            (*it)->Draw(shader, projection, view);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    cout << "Shutting down Melonyx Engine" << endl;
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}