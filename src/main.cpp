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

#include "headers/anchoredspring.h"
#include "headers/rod.h"

#include <cstdlib>

using namespace std;
using namespace chrono_literals;

// ===== WINDOW =====
const int    WINDOW_SIZE = 800;
const string WINDOW_TITLE = "Erica Mauriz Barundia";

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

    // Create particle
    melonyx::Particle p1;
    p1.Position = glm::vec3(0.0f, 50.0f, 0.0f);
    p1.mass = 50.0f;
    p1.damping = 0.9f;
    p1.Velocity = glm::vec3(0.0f);
    p1.Acceleration = glm::vec3(0.0f);
    pWorld.AddParticle(&p1);

    // Add force: (0.6, 0.3, 0) as per assignment
    p1.AddForce(glm::vec3(0.6f, 0.3f, 0.0f) * 1000000.0f);

    melonyx::Particle p2;
    p2.Position = glm::vec3(0.0f, 100.0f, 0.0f);
    p2.mass = 50.0f;
    p2.damping = 0.9f;
    p2.Velocity = glm::vec3(0.0f);
    p2.Acceleration = glm::vec3(0.0f);
    pWorld.AddParticle(&p2);


    // Create render particle linked to p1 and sphere
    RenderParticle rp1(&p1, &sphere, glm::vec3(1.0f, 0.0f, 0.0f)); // red
    
    RenderParticle rp2(&p2, &sphere, glm::vec3(0.0f, 1.0f, 0.0f)); // green

    // Rods
    melonyx::Rod* r = new melonyx::Rod();
    r->particles[0] = &p1;
    r->particles[1] = &p2;
    r->length = 50; // matches initial 50-unit gap between p1 and p2

    pWorld.Links.push_back(r);


    // Gravity: (0, -9.8, 0)
    GravityForceGenerator gravity(glm::vec3(0.0f, -9.8f, 0.0f));
    pWorld.forceRegistry.Add(&p1, &gravity);

    // Drag
    DragForceGenerator drag(0.14f, 0.1f);
    pWorld.forceRegistry.Add(&p1, &drag);

    // Anchored spring: anchor above, spring constant = 5, rest length = 0.5
    glm::vec3 springAnchor = glm::vec3(0.0f, 200.0f, 0.0f);
    melonyx::AnchoredSpring aSpring(springAnchor, 50.0f, 0.5f);
    pWorld.forceRegistry.Add(&p1, &aSpring);
    pWorld.forceRegistry.Add(&p2, &aSpring);

    std::list<RenderParticle*> RenderParticles;
    RenderParticles.push_back(&rp1);
    RenderParticles.push_back(&rp2);


    // ===== SPRING LINE VAO (persistent, updated each frame) =====
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    // Allocate space for 2 vertices * 3 floats, dynamic since it updates every frame
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ===== ROD LINE VAO (persistent, updated each frame) =====
    GLuint rodLineVAO, rodLineVBO;
    glGenVertexArrays(1, &rodLineVAO);
    glGenBuffers(1, &rodLineVBO);
    glBindVertexArray(rodLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rodLineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

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
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        float     orthoSize = 350.0f;
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

        // Draw particles
        for (auto it = RenderParticles.begin(); it != RenderParticles.end(); ++it)
            (*it)->Draw(shader, projection, view);

        // --- Draw spring line ---
        {
            float lineVerts[] = {
                springAnchor.x, springAnchor.y, springAnchor.z,
                p1.Position.x,  p1.Position.y,  p1.Position.z
            };

            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVerts), lineVerts);

            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3f(glGetUniformLocation(shader, "color"), 1.0f, 1.0f, 1.0f); // white

            glBindVertexArray(lineVAO);
            glDrawArrays(GL_LINES, 0, 2);
            glBindVertexArray(0);
        }

        // --- Draw rod line ---
        {
            float rodVerts[] = {
                p1.Position.x, p1.Position.y, p1.Position.z,
                p2.Position.x, p2.Position.y, p2.Position.z
            };

            glBindBuffer(GL_ARRAY_BUFFER, rodLineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rodVerts), rodVerts);

            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3f(glGetUniformLocation(shader, "color"), 1.0f, 1.0f, 0.0f); // yellow

            glBindVertexArray(rodLineVAO);
            glDrawArrays(GL_LINES, 0, 2);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    cout << "Shutting down Melonyx Engine" << endl;
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &rodLineVAO);
    glDeleteBuffers(1, &rodLineVBO);
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}