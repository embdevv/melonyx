/**
 * @file main.cpp
 * @author Erica Mauriz Barundia
 *
 * Phase 2: Mass Aggregate - Newton's Cradle Demo
 * ------------------------------------------------
 * - Gravity applied via Force Generator (PhysicsWorld::SetGravity)
 * - 5 particles, each joined to a fixed anchor point via a Cable
 * - Particle-particle collision handled automatically by PhysicsWorld
 * - User inputs simulation parameters at the console before the window opens
 * - Space: launches the left-most particle with the inputted force
 * - 1 / 2: switch between orthographic (2D-style) and perspective (3D) view
 * - WASD: rotate camera (perspective view only)
 * - ESC: quit
 */

#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cmath>

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

// ===== WINDOW =====
const int    WINDOW_SIZE = 800;
const string WINDOW_TITLE = "melonYX engine";

// ===== NUMBER OF PARTICLES IN THE CRADLE =====
const int NUM_PARTICLES = 5;

// ===== SIMULATION PARAMETERS (filled in from console input) =====
struct SimParams {
    float cableLength;
    float particleGap;
    float particleRadius;
    float gravityStrength; // Y-axis gravity (magnitude, will be applied as negative Y)
    glm::vec3 applyForce;      // force applied to left-most particle on Space
};

// ===== CAMERA STATE =====
struct CameraState {
    int   viewMode = 1;      // 1 = orthographic, 2 = perspective
    float yaw = 0.0f;        // radians, rotate around Y
    float pitch = 0.3f;      // radians, clamp to avoid flipping
    float distance = 700.0f; // orbit distance for perspective view
};

// ===== APP CONTEXT (used for key callback + Space launch) =====
struct AppContext {
    melonyx::Particle* leftMostParticle = nullptr;
    glm::vec3 force = glm::vec3(0.0f);
};

static SimParams ReadSimParams()
{
    SimParams p{};
    cout << "===== Newton's Cradle Setup =====\n";

    cout << "Cable Length: ";
    cin >> p.cableLength;

    cout << "Particle Gap (distance between particle centers): ";
    cin >> p.particleGap;

    cout << "Particle Radius: ";
    cin >> p.particleRadius;

    cout << "Gravity Strength (Y-axis): ";
    cin >> p.gravityStrength;

    cout << "Apply Force (applied to left-most particle when you press Space): ";
    cout << "x: ";
    cin >> p.applyForce.x;
    cout << "y: ";
    cin >> p.applyForce.y;
    cout << "z: ";
    cin >> p.applyForce.z;


    cout << "==================================\n";
    cout << "Setup complete. Press Space in the window to launch!\n";

    return p;
}

// ===== KEY CALLBACK (edge-triggered events: ESC + Space) =====
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
        if (ctx && ctx->leftMostParticle)
        {
            // One-shot force applied on the frame Space is pressed.
            // Particle::ResetForce() clears accumulated force every Update(),
            // so this behaves like a launch impulse, not a sustained push.
            ctx->leftMostParticle->AddForce(ctx->force);
        }
    }
}

int main()
{
    SimParams params = ReadSimParams();

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

    // Build sphere (model provided by the assignment)
    ObjMesh sphere;
    if (!sphere.load("3D/sphere.obj"))
    {
        cerr << "Failed to load sphere.obj" << endl;
        glfwTerminate();
        return -1;
    }
    sphere.upload();

    // OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);

    // ===== PHYSICS WORLD SETUP =====
    melonyx::PhysicsWorld pWorld = melonyx::PhysicsWorld();

    // Gravity Strength is user-configurable in the Y-axis (must be set
    // before AddParticle, since particles register the world's gravity
    // generator immediately when added)
    pWorld.SetGravity(glm::vec3(0.0f, params.gravityStrength, 0.0f));

    // ===== BUILD THE CRADLE: 5 particles, each on its own cable =====
    // Anchor height = cable length, so at REST every particle hangs at y = 0,
    // keeping the cradle centered vertically in the 800x800 window.
    // Anchor X spacing = particleGap, centered so the 3rd particle (index 2)
    // sits at x = 0 at the start, per spec.
    const float anchorY = params.cableLength;

    vector<melonyx::Particle>     particles(NUM_PARTICLES);
    vector<glm::vec3>              anchors(NUM_PARTICLES);
    vector<melonyx::Cable*>        cables(NUM_PARTICLES);
    vector<RenderParticle*>        renderParticles(NUM_PARTICLES);

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        float anchorX = (i - (NUM_PARTICLES / 2)) * params.particleGap;
        anchors[i] = glm::vec3(anchorX, anchorY, 0.0f);

        // Particles as per spec
        particles[i].mass = 50.0f;
        particles[i].restitution = 0.9f;
        particles[i].radius = params.particleRadius;
        particles[i].damping = 0.999f;

        // Start right at the anchor (slack cable) so they visibly
        // "drop down" under gravity until the cable goes taut.
        particles[i].Position = anchors[i] + glm::vec3(0.0f, -1.0f, 0.0f);
        particles[i].Velocity = glm::vec3(0.0f);
        particles[i].Acceleration = glm::vec3(0.0f);

        pWorld.AddParticle(&particles[i]);

        // Cable: anchors particle to a fixed point, never exceeds cableLength
        melonyx::Cable* cable = new melonyx::Cable(anchors[i], params.cableLength, 0.9f);
        cable->particle = &particles[i];
        cables[i] = cable;
        pWorld.Cables.push_back(cable);

        renderParticles[i] = new RenderParticle(&particles[i], &sphere, glm::vec3(0.85f, 0.15f, 0.15f));
    }

    // ===== APP CONTEXT for key callback (Space launches left-most particle) =====
    AppContext appCtx;
    appCtx.leftMostParticle = &particles[0];
    appCtx.force = params.applyForce;
    glfwSetWindowUserPointer(window, &appCtx);
    glfwSetKeyCallback(window, KeyCallback);

    // ===== CABLE LINE VAO (persistent, updated each frame, reused per cable) =====
    GLuint cableLineVAO, cableLineVBO;
    glGenVertexArrays(1, &cableLineVAO);
    glGenBuffers(1, &cableLineVBO);
    glBindVertexArray(cableLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cableLineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ===== CAMERA =====
    CameraState camera;
    const float orthoSize = WINDOW_SIZE / 2.0f; // 400, matches 1m:1px scale for the 800x800 window
    const glm::vec3 orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    const float camRotateSpeed = 1.5f; // radians/sec

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
        float frameDt = dur.count() / (float)(1E09);

        // --- View switching (1/2) ---
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) camera.viewMode = 1;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) camera.viewMode = 2;

        // --- Camera rotation (WASD), only meaningful in perspective view ---
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.yaw -= camRotateSpeed * frameDt;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.yaw += camRotateSpeed * frameDt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.pitch += camRotateSpeed * frameDt;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.pitch -= camRotateSpeed * frameDt;
        camera.pitch = glm::clamp(camera.pitch, -1.5f, 1.5f);

        // --- Physics update (fixed timestep) ---
        while (curr_ns >= timestep) {
            curr_ns -= timestep;
            pWorld.Update(timestep_sec);
        }

        // --- Build projection/view for the active camera mode ---
        glm::mat4 projection, view;
        if (camera.viewMode == 1)
        {
            // Orthographic front view (2D-style), 1m:1px scale
            projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 2000.0f);
            view = glm::lookAt(
                glm::vec3(0.0f, 0.0f, 500.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
        }
        else
        {
            // Perspective view, orbit-able with WASD
            projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 3000.0f);
            glm::vec3 eye = orbitCenter + camera.distance * glm::vec3(
                cosf(camera.pitch) * sinf(camera.yaw),
                sinf(camera.pitch),
                cosf(camera.pitch) * cosf(camera.yaw)
            );
            view = glm::lookAt(eye, orbitCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        // Draw particles
        for (int i = 0; i < NUM_PARTICLES; i++)
            renderParticles[i]->Draw(shader, projection, view);

        // Draw cable lines (anchor -> particle), one draw call per cable
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glUniform3f(glGetUniformLocation(shader, "color"), 1.0f, 1.0f, 1.0f); // white

        for (int i = 0; i < NUM_PARTICLES; i++)
        {
            float lineVerts[] = {
                anchors[i].x, anchors[i].y, anchors[i].z,
                particles[i].Position.x, particles[i].Position.y, particles[i].Position.z
            };

            glBindBuffer(GL_ARRAY_BUFFER, cableLineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVerts), lineVerts);

            glBindVertexArray(cableLineVAO);
            glDrawArrays(GL_LINES, 0, 2);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    cout << "Shutting down Melonyx Engine" << endl;
    glDeleteVertexArrays(1, &cableLineVAO);
    glDeleteBuffers(1, &cableLineVBO);
    for (int i = 0; i < NUM_PARTICLES; i++)
        delete renderParticles[i];
    // Cables are owned by pWorld.Cables; no explicit cleanup path existed
    // before either, consistent with the engine's current lifetime model.
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}