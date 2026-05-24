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

#include <cstdlib>

using namespace std;
using namespace chrono_literals;

// ===== WINDOW =====
const int    WINDOW_SIZE  = 800;
const string WINDOW_TITLE = "Erica Mauriz Barundia";

// ===== MAIN =====
int main()
{
    constexpr chrono::nanoseconds timestep(16ms);

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
    Sphere sphere;
    sphere.build(1.0f, 72, 36);
    sphere.upload();
    sphere.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sphere.color    = glm::vec3(0.8f, 0.1f, 0.1f);

    // OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);

    // ESC to close
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    // Particle
    melonyx::Particle particle;
    particle.Position = glm::vec3(0.0f, 0.0f, 0.0f); // start left of center
    particle.Velocity = glm::vec3(10.0f, 0.0f, 0.0f);  // slow enough to see
    particle.Acceleration = glm::vec3(0.0f, 0.0f, 0.0f);  // decelerate to stop

    
    std::list<RenderParticle*> RenderParticles;
    RenderParticle rp1 = RenderParticle(
        &particle, 
        &sphere, 
        glm::vec3(0.4f, 0.1f, 0.1f)
    );
    RenderParticles.push_back(&rp1);

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
            constexpr float timestep_sec = timestep.count() / (float)(1E09);
            curr_ns -= timestep;
            particle.Update(timestep_sec);
            sphere.position = particle.Position;

            // limit the particle to the window bounds
            const float boundary = 5.0f - 1.0f;
            
            if (particle.Position.x >= boundary || particle.Position.x <= -boundary) {
                particle.Position.x = glm::clamp(particle.Position.x, -boundary, boundary);
                particle.Velocity.x *= -1.0f;
            }

            cout << "Physics Update: Particle position: " << particle.Position.x << ", " << particle.Position.y << ", " << particle.Position.z << endl;
        }
        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        float orthoSize = 5.0f;
        glm::mat4 projection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize, orthoSize,
            0.1f, 100.0f
        );

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),
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