/**
 * @file main.cpp
 * @author Erica Mauriz Barundia
 *
 * Phase 2: Programming Challenge 2 - Prize Roulette Demo
 * ------------------------------------------------
 * Design note: the wheel is a single ROTATIONAL degree of freedom, not
 * 5 independently-simulated point masses. So instead of faking circular
 * motion with 5 separately-cabled particles (like the Newton's Cradle
 * demo did), a single melonyx::Particle ("wheelState") represents the
 * wheel's spin: Position.x = current angle (radians), Velocity.x =
 * angular velocity. It still goes through the engine's real integrator
 * (UpdatePosition / UpdateVelocity / damping via Particle::Update), just
 * applied to a rotational quantity instead of a linear one. The 5 colored
 * spheres are then rendered kinematically at their angular offset from
 * that single wheel angle.
 *
 * - 5 particles, 50kg each, distinct colors, each mapped to a prize
 * - Legend (color -> prize) is printed to console BEFORE the window opens
 * - User then inputs a 2D force (Fx, Fy), applied as a torque to whichever
 *   particle is on top at that instant (tau = rx*Fy - ry*Fx)
 * - Wheel spins and slows down naturally via damping
 * - Holding Space applies extra braking so it slows down faster
 * - Once angular velocity settles near zero, the sim freezes and waits
 *   for Enter in the console before printing the winning prize and exiting
 * - Perspective projection only, fixed camera framing the whole wheel
 * - ESC: quit early
 */

#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <cmath>
#include <limits>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headers/main.h"
#include "headers/shader.h"
#include "headers/particle.h"
#include "headers/render_particle.h"

using namespace std;
using namespace chrono_literals;

// ===== WINDOW =====
const int    WINDOW_SIZE = 800;
const string WINDOW_TITLE = "PC02 Barundia";

// ===== WHEEL LAYOUT =====
const int   NUM_PARTICLES  = 5;
const float WHEEL_RADIUS   = 220.0f;
const float PARTICLE_RADIUS= 35.0f;
const float PARTICLE_MASS  = 50.0f;

// Natural (passive) spin-down damping, applied every physics tick regardless
// of input, same mechanism as Particle::damping used in the Cradle demo.
// NOTE: Particle::Update applies this as velocity *= powf(damping, dt), which
// compounds continuously -- so `damping` is literally "fraction of velocity
// remaining after 1 full second." 0.6 means it decays to a natural stop over
// roughly 8-12 seconds for a typical test spin.
const float WHEEL_DAMPING = 0.6f;

// Extra multiplicative decay applied ON TOP of WHEEL_DAMPING while Space is
// held, so the wheel brakes noticeably faster than letting it stop on its own.
// 0.05 means velocity drops to ~5% within about 1 second of holding Space.
const float BRAKE_DAMPING = 0.05f;

// Angular velocity magnitude below which the wheel is considered "stopped".
const float STOP_THRESHOLD = 0.01f;
// Consecutive frames it must stay below threshold before we trust it (debounce).
const int   STOP_DEBOUNCE_FRAMES = 20;

// ===== PRIZE TABLE (color -> prize) =====
struct PrizeSlot {
    string    colorName;
    glm::vec3 color;
    string    prize;
};

const vector<PrizeSlot> PRIZES = {
    { "Ruby",     glm::vec3(0.75f, 0.05f, 0.20f), "50 Credits"     },
    { "Amber",    glm::vec3(0.85f, 0.50f, 0.05f), "Extra Life"     },
    { "Citrine",  glm::vec3(0.80f, 0.75f, 0.10f), "Speed Boost"    },
    { "Emerald",  glm::vec3(0.05f, 0.55f, 0.30f), "Shield Charge"  },
    { "Sapphire", glm::vec3(0.10f, 0.25f, 0.70f), "JACKPOT SPIN"   },
};

// Base angular offset of particle i around the wheel, spaced 360/5 = 72
// degrees apart, with particle 0 starting exactly at the top (90 deg / pi/2).
static float BaseAngle(int i)
{
    return glm::half_pi<float>() + i * (glm::two_pi<float>() / (float)NUM_PARTICLES);
}

// Normalize an angle into (-pi, pi]
static float NormalizeAngle(float a)
{
    a = fmodf(a, glm::two_pi<float>());
    if (a > glm::pi<float>())  a -= glm::two_pi<float>();
    if (a < -glm::pi<float>()) a += glm::two_pi<float>();
    return a;
}

// Which particle index is currently closest to "top" (angle = pi/2)?
static int GetTopIndex(float wheelAngle)
{
    int   best = 0;
    float bestDiff = std::numeric_limits<float>::max();
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        float theta = BaseAngle(i) + wheelAngle;
        float diff = fabsf(NormalizeAngle(theta - glm::half_pi<float>()));
        if (diff < bestDiff) { bestDiff = diff; best = i; }
    }
    return best;
}

// ===== APP CONTEXT (only ESC needs a callback here; Space is polled) =====
struct AppContext {
    bool escPressed = false;
};

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
    // ===== BEFORE THE SIMULATION: print the prize legend =====
    cout << "+--------------------------------------+\n";
    cout << "|         M E L O N Y X  S P I N        |\n";
    cout << "+--------------------------------------+\n";
    cout << "  Gem       Prize\n";
    cout << "  --------  --------------\n";
    for (auto& slot : PRIZES)
        cout << "  " << slot.colorName << string(10 - slot.colorName.size(), ' ') << slot.prize << "\n";

    // ===== BEFORE THE SIMULATION: read the 2D force to apply =====
    glm::vec2 inputForce{};
    cout << "\nGive the wheel a spin -- enter a 2D force:\n";
    cout << "x: ";
    cin >> inputForce.x;
    cout << "y: ";
    cin >> inputForce.y;

    cout << "+--------------------------------------+\n";
    cout << "  Spinning up... hold SPACE to brake.\n";
    cout << "  ESC to quit early.\n";

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

    if (!gladLoadGL()) {
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

    // ===== WHEEL STATE (single rotational particle, engine-driven) =====
    // Not added to a PhysicsWorld: there's no gravity or particle-particle
    // collision here, just one rotational DOF using the engine's own
    // Particle::Update integrator (mass here = moment-of-inertia analog).
    melonyx::Particle wheelState;
    wheelState.mass    = PARTICLE_MASS * NUM_PARTICLES; // heavier wheel = tamer spin per unit force
    wheelState.damping = WHEEL_DAMPING;
    wheelState.Position = glm::vec3(0.0f);
    wheelState.Velocity = glm::vec3(0.0f);
    wheelState.Acceleration = glm::vec3(0.0f);

    // Apply the user's force as a torque impulse to the top-most particle,
    // right as the simulation starts (angle = 0, so GetTopIndex(0) applies).
    {
        int topIdx = GetTopIndex(wheelState.Position.x);
        float theta = BaseAngle(topIdx);
        glm::vec2 r(cosf(theta) * WHEEL_RADIUS, sinf(theta) * WHEEL_RADIUS);
        float torque = r.x * inputForce.y - r.y * inputForce.x;
        wheelState.AddForce(glm::vec3(torque, 0.0f, 0.0f));
    }

    // ===== RENDER PARTICLES (kinematic: positioned each frame from wheel angle) =====
    vector<melonyx::Particle> particles(NUM_PARTICLES);
    vector<RenderParticle*>   renderParticles(NUM_PARTICLES);

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particles[i].mass        = PARTICLE_MASS;
        particles[i].radius      = PARTICLE_RADIUS;
        particles[i].restitution = 0.9f;

        float theta = BaseAngle(i) + wheelState.Position.x;
        particles[i].Position = glm::vec3(cosf(theta) * WHEEL_RADIUS, sinf(theta) * WHEEL_RADIUS, 0.0f);

        renderParticles[i] = new RenderParticle(&particles[i], &sphere, PRIZES[i].color);
    }

    AppContext appCtx;
    glfwSetWindowUserPointer(window, &appCtx);
    glfwSetKeyCallback(window, KeyCallback);

    // ===== ROD LINE VAO (persistent, updated each frame, reused per spoke) =====
    // Visual spokes from the wheel's center to each particle, same technique
    // as the Cradle demo's cable-line rendering.
    const glm::vec3 wheelCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    GLuint rodLineVAO, rodLineVBO;
    glGenVertexArrays(1, &rodLineVAO);
    glGenBuffers(1, &rodLineVBO);
    glBindVertexArray(rodLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rodLineVBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // ===== CAMERA: fixed perspective, framed so the whole wheel stays visible =====
    const glm::vec3 orbitCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    const glm::vec3 cameraEye   = glm::vec3(0.0f, 0.0f, 700.0f);
    const glm::mat4 projection  = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 3000.0f);
    const glm::mat4 view        = glm::lookAt(cameraEye, orbitCenter, glm::vec3(0.0f, 1.0f, 0.0f));

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    int  stoppedFrames = 0;
    bool wheelStopped  = false;

    // ===== RENDER LOOP =====
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // --- Frame timing ---
        curr_time = clock::now();
        auto dur = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
        curr_ns += dur;

        if (!wheelStopped)
        {
            bool braking = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

            // --- Physics update (fixed timestep) ---
            while (curr_ns >= timestep)
            {
                curr_ns -= timestep;
                wheelState.Update(timestep_sec);

                // Extra braking on top of the wheel's passive damping.
                if (braking)
                    wheelState.Velocity.x *= powf(BRAKE_DAMPING, timestep_sec);

                // Debounced "has it basically stopped?" check.
                if (fabsf(wheelState.Velocity.x) < STOP_THRESHOLD)
                {
                    stoppedFrames++;
                    if (stoppedFrames >= STOP_DEBOUNCE_FRAMES)
                    {
                        wheelState.Velocity.x = 0.0f;
                        wheelStopped = true;
                    }
                }
                else
                {
                    stoppedFrames = 0;
                }
            }

            // --- Update rendered particle positions from the wheel angle ---
            for (int i = 0; i < NUM_PARTICLES; i++)
            {
                float theta = BaseAngle(i) + wheelState.Position.x;
                particles[i].Position = glm::vec3(cosf(theta) * WHEEL_RADIUS, sinf(theta) * WHEEL_RADIUS, 0.0f);
            }
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        for (int i = 0; i < NUM_PARTICLES; i++)
            renderParticles[i]->Draw(shader, projection, view);

        // Draw rod spokes (center -> particle), one draw call per rod
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glUniform3f(glGetUniformLocation(shader, "color"), 1.0f, 1.0f, 1.0f); // white

        for (int i = 0; i < NUM_PARTICLES; i++)
        {
            float rodVerts[] = {
                wheelCenter.x, wheelCenter.y, wheelCenter.z,
                particles[i].Position.x, particles[i].Position.y, particles[i].Position.z
            };

            glBindBuffer(GL_ARRAY_BUFFER, rodLineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(rodVerts), rodVerts);

            glBindVertexArray(rodLineVAO);
            glDrawArrays(GL_LINES, 0, 2);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);

        // Once the wheel has settled, break out and go wait on the console.
        if (wheelStopped)
            break;
    }

    // ===== WAIT FOR ENTER, THEN PRINT THE RESULT AND EXIT =====
    if (wheelStopped && !glfwWindowShouldClose(window))
    {
        cout << "\nThe wheel has stopped. Press Enter to reveal your prize...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cin.get();

        int winner = GetTopIndex(wheelState.Position.x);
        cout << "\nResult: " << PRIZES[winner].colorName
             << " -> You won: " << PRIZES[winner].prize << "\n";

        // Keep the console open until the user explicitly dismisses it --
        // otherwise, on Windows especially, launching the .exe directly
        // (not from an already-open terminal) closes the console the
        // instant main() returns, before the result is even readable.
        cout << "\nPress Enter to exit...";
        cin.get();
    }

    // Cleanup
    cout << "Shutting down Melonyx Engine" << endl;
    glDeleteVertexArrays(1, &rodLineVAO);
    glDeleteBuffers(1, &rodLineVBO);
    for (int i = 0; i < NUM_PARTICLES; i++)
        delete renderParticles[i];
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}