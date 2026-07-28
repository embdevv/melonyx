/**
 * @file main.cpp
 * @author Erica Mauriz Barundia
 *
 * Dead Men Drink No Rum
 * Milestone 3: Cannon + Aiming
 */

#include <iostream>
#include <string>
#include <chrono>
#include <vector>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headers/constants.h"
#include "headers/game_state.h"
#include "headers/level.h"
#include "headers/particle.h"
#include "headers/world_particle.h"
#include "headers/force_gen.h"
#include "headers/force_registry.h"
#include "headers/gravity_force_gen.h"
#include "headers/drag_force_gen.h"
#include "headers/bottle.h"
#include "headers/collision.h"
#include "headers/cannon.h"
#include "headers/shader.h"

using namespace std;
using namespace chrono_literals;

// ───────────────────────────────────────────────────────────────────────────
// Window
// ───────────────────────────────────────────────────────────────────────────
const int    WINDOW_SIZE  = 800;
const string WINDOW_TITLE = "Dead Men Drink No Rum";

// ───────────────────────────────────────────────────────────────────────────
// Game state
// ───────────────────────────────────────────────────────────────────────────
GameState   gState          = GameState::AIMING;
int         gCurrentLevel   = 1;
int         gBottlesTotal   = 0;
int         gBottlesThisRun = 0;
LevelConfig gLevel          = getLevelConfig(1);

// ───────────────────────────────────────────────────────────────────────────
// Physics
// ───────────────────────────────────────────────────────────────────────────
melonyx::PhysicsWorld pWorld;
melonyx::Particle     pirate;
GravityForceGenerator gravityGen(glm::vec3(0.0f, -GRAVITY, 0.0f));
DragForceGenerator    dragGen(DRAG_K1, DRAG_K2);

// ───────────────────────────────────────────────────────────────────────────
// Cannon — sits bottom-left of screen
// ───────────────────────────────────────────────────────────────────────────
pirategame::Cannon gCannon(-350.0f, -350.0f);

// ───────────────────────────────────────────────────────────────────────────
// Bottles
// ───────────────────────────────────────────────────────────────────────────
vector<pirategame::Bottle> gBottles;

void loadLevelBottles(int level)
{
    gBottles.clear();
    switch (level) {
        case 1:
            gBottles.emplace_back( 100.0f,  -50.0f);
            gBottles.emplace_back( 250.0f, -100.0f);
            gBottles.emplace_back( 350.0f,  -80.0f);
            break;
        case 2:
            gBottles.emplace_back( 150.0f,  -60.0f);
            gBottles.emplace_back( 200.0f,  -60.0f);
            gBottles.emplace_back( 320.0f, -120.0f);
            gBottles.emplace_back( 380.0f,  -90.0f);
            break;
        case 3:
            gBottles.emplace_back(  80.0f,  -50.0f);
            gBottles.emplace_back( 120.0f,  -50.0f);
            gBottles.emplace_back( 100.0f,  -90.0f);
            gBottles.emplace_back( 280.0f, -100.0f);
            gBottles.emplace_back( 320.0f, -100.0f);
            gBottles.emplace_back( 300.0f, -140.0f);
            break;
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Pirate reset
// ───────────────────────────────────────────────────────────────────────────
void resetPirate()
{
    // Spawn at cannon muzzle
    glm::vec3 muzzle    = gCannon.muzzlePosition();
    pirate.Position     = muzzle;
    pirate.Velocity     = glm::vec3(0.0f);
    pirate.Acceleration = glm::vec3(0.0f);
    pirate.mass         = 1.0f;
    pirate.damping      = 0.999f;
    pirate.radius       = PIRATE_RADIUS;
    pirate.ResetForce();
}

// ───────────────────────────────────────────────────────────────────────────
// Level / state transitions
// ───────────────────────────────────────────────────────────────────────────
void startLevel(int level)
{
    gCurrentLevel = level;
    gLevel        = getLevelConfig(level);
    loadLevelBottles(level);
    resetPirate();
    gState          = GameState::AIMING;
    gBottlesThisRun = 0;

    // Reset physics world for new level
    pWorld = melonyx::PhysicsWorld();

    cout << "[LEVEL] Level " << level
         << " | quota: " << gLevel.bottleQuota << "\n";
}

void launchPirate()
{
    resetPirate();
    pWorld = melonyx::PhysicsWorld();

    pirate.Velocity = gCannon.fire();  // ← real cannon velocity

    pWorld.AddParticle(&pirate);
    pWorld.forceRegistry.Add(&pirate, &gravityGen);
    pWorld.forceRegistry.Add(&pirate, &dragGen);

    gState = GameState::LAUNCHED;
    cout << "[LAUNCH] angle=" << gCannon.angleDeg
         << "deg  power=" << gCannon.power
         << "  vel=(" << pirate.Velocity.x
         << ", "      << pirate.Velocity.y << ")\n";
}

void onFlightEnd(bool drowned)
{
    gBottlesThisRun = pirategame::bottlesCollected(gBottles);
    cout << "[RESULT] Bottles: " << gBottlesThisRun << "\n";

    if (drowned || gBottlesThisRun == 0) {
        gState        = GameState::GAME_OVER;
        gBottlesTotal = 0;
        startLevel(1);
        cout << "[STATE] GAME_OVER — reset\n";
        return;
    }

    gBottlesTotal += gBottlesThisRun;

    if (gCurrentLevel < 3) {
        startLevel(gCurrentLevel + 1);
    } else {
        if (gBottlesTotal >= TOTAL_QUOTA) {
            gState = GameState::WIN;
            cout << "[STATE] WIN — " << gBottlesTotal << " bottles total\n";
        } else {
            gState        = GameState::GAME_OVER;
            gBottlesTotal = 0;
            startLevel(1);
            cout << "[STATE] GAME_OVER — quota missed\n";
        }
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Trajectory preview — simulates arc without touching physics world
// ───────────────────────────────────────────────────────────────────────────
vector<glm::vec2> computeTrajectory()
{
    vector<glm::vec2> points;
    glm::vec2 pos(gCannon.muzzlePosition());
    glm::vec2 vel(gCannon.fire());

    for (int i = 0; i < PREVIEW_STEPS; i++) {
        vel.y -= GRAVITY * PREVIEW_DT;
        vel   *= (1.0f - DRAG_K1 * PREVIEW_DT); // approximate drag
        pos   += vel * PREVIEW_DT;
        points.push_back(pos);

        // Stop preview at water line
        if (pos.y < gLevel.waterLine) break;
    }
    return points;
}

// ───────────────────────────────────────────────────────────────────────────
// Input
// ───────────────────────────────────────────────────────────────────────────
static void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (gState == GameState::AIMING)
        {
            if (key == GLFW_KEY_LEFT)  { gCannon.rotateLeft();  cout << "[AIM] angle=" << gCannon.angleDeg << "\n"; }
            if (key == GLFW_KEY_RIGHT) { gCannon.rotateRight(); cout << "[AIM] angle=" << gCannon.angleDeg << "\n"; }
            if (key == GLFW_KEY_UP)    { gCannon.powerUp();     cout << "[AIM] power=" << gCannon.power    << "\n"; }
            if (key == GLFW_KEY_DOWN)  { gCannon.powerDown();   cout << "[AIM] power=" << gCannon.power    << "\n"; }
            if (key == GLFW_KEY_SPACE) launchPirate();
            if (key == GLFW_KEY_R)     startLevel(gCurrentLevel);
        }

        if (gState == GameState::GAME_OVER || gState == GameState::WIN)
        {
            if (key == GLFW_KEY_R) startLevel(1);
        }
    }
}

// ───────────────────────────────────────────────────────────────────────────
// OpenGL helpers — simple colored point/dot for debug rendering
// ───────────────────────────────────────────────────────────────────────────
// We'll draw dots for trajectory preview and bottles using GL_POINTS
// Full sprite rendering comes in Milestone 4 with the pirate mesh

GLuint gShader  = 0;
GLuint gDotVAO  = 0;
GLuint gDotVBO  = 0;

void initDotRenderer()
{
    glGenVertexArrays(1, &gDotVAO);
    glGenBuffers(1, &gDotVBO);
    glBindVertexArray(gDotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gDotVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void drawDot(const glm::vec2& pos, const glm::mat4& proj, const glm::mat4& view,
             float r, float g, float b, float size = 8.0f)
{
    float v[3] = { pos.x, pos.y, 0.0f };
    glBindBuffer(GL_ARRAY_BUFFER, gDotVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(gShader, "model"),      1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(gShader, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gShader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3f(glGetUniformLocation(gShader, "color"), r, g, b);

    glPointSize(size);
    glBindVertexArray(gDotVAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}

// ───────────────────────────────────────────────────────────────────────────
// Main
// ───────────────────────────────────────────────────────────────────────────
int main()
{
    constexpr chrono::nanoseconds timestep(16ms);
    constexpr float timestep_sec = timestep.count() / (float)(1E09);

    if (!glfwInit()) { cerr << "GLFW init failed\n"; return -1; }

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_SIZE, WINDOW_SIZE, WINDOW_TITLE.c_str(), NULL, NULL);
    if (!window) { cerr << "Window failed\n"; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        cerr << "GLAD failed\n"; glfwTerminate(); return -1;
    }

    cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";

    gShader = compileShaders("shaders/sample.vert", "shaders/sample.frag");
    if (!gShader) { cerr << "Shader failed\n"; glfwTerminate(); return -1; }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(0.05f, 0.12f, 0.25f, 1.0f);

    initDotRenderer();
    startLevel(1);

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        curr_time = clock::now();
        auto dur  = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
        curr_ns  += dur;

        while (curr_ns >= timestep)
        {
            curr_ns -= timestep;

            if (gState == GameState::LAUNCHED)
            {
                pWorld.Update(timestep_sec);

                auto result = pirategame::checkCollisions(
                    pirate, gBottles, gLevel.waterLine);

                if (result.hitBottle)
                    cout << "[HIT] " << result.bottlesHit << " bottle(s)\n";

                bool outOfBounds = pirate.Position.x >  ORTHO_SIZE + 50.0f
                                || pirate.Position.x < -ORTHO_SIZE - 50.0f;

                if (result.hitWater || outOfBounds)
                    onFlightEnd(result.hitWater);
            }
        }

        // ── Render ───────────────────────────────────────────────────
        switch (gState) {
            case GameState::AIMING:    glClearColor(0.05f, 0.12f, 0.25f, 1.0f); break;
            case GameState::LAUNCHED:  glClearColor(0.08f, 0.18f, 0.30f, 1.0f); break;
            case GameState::RESULT:    glClearColor(0.12f, 0.10f, 0.05f, 1.0f); break;
            case GameState::GAME_OVER: glClearColor(0.20f, 0.02f, 0.02f, 1.0f); break;
            case GameState::WIN:       glClearColor(0.10f, 0.20f, 0.05f, 1.0f); break;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(gShader);

        const float orthoSize = ORTHO_SIZE;
        glm::mat4 proj = glm::ortho(
            -orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, 500), glm::vec3(0), glm::vec3(0, 1, 0));

        if (gState == GameState::AIMING)
        {
            // Draw trajectory preview — white dots
            auto traj = computeTrajectory();
            for (auto& pt : traj)
                drawDot(pt, proj, view, 1.0f, 1.0f, 1.0f, 4.0f);

            // Draw cannon position — yellow dot
            drawDot(gCannon.position, proj, view, 1.0f, 0.8f, 0.0f, 16.0f);

            // Draw muzzle tip — orange dot
            glm::vec2 muzzle(gCannon.muzzlePosition());
            drawDot(muzzle, proj, view, 1.0f, 0.5f, 0.0f, 10.0f);
        }

        if (gState == GameState::LAUNCHED)
        {
            // Draw pirate — red dot
            glm::vec2 piratePos(pirate.Position.x, pirate.Position.y);
            drawDot(piratePos, proj, view, 1.0f, 0.1f, 0.1f, 20.0f);
        }

        // Draw bottles — green = uncollected, grey = collected
        for (auto& b : gBottles)
        {
            if (b.collected)
                drawDot(b.position, proj, view, 0.4f, 0.4f, 0.4f, 14.0f);
            else
                drawDot(b.position, proj, view, 0.1f, 0.9f, 0.2f, 14.0f);
        }

        // Draw water line — blue horizontal dots
        for (float x = -orthoSize; x <= orthoSize; x += 20.0f)
            drawDot(glm::vec2(x, gLevel.waterLine), proj, view, 0.1f, 0.3f, 0.9f, 4.0f);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &gDotVAO);
    glDeleteBuffers(1, &gDotVBO);
    glDeleteProgram(gShader);
    glfwTerminate();
    return 0;
}