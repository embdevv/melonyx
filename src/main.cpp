/**
 * @file main.cpp
 * @author Erica Mauriz Barundia
 *
 * Dead Men Drink No Rum
 * Milestone 4: Pirate + Rotation + Mesh Rendering
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

#include "headers/pirate/constants.h"
#include "headers/pirate/game_state.h"
#include "headers/pirate/level.h"
#include "headers/pirate/bottle.h"
#include "headers/pirate/collision.h"
#include "headers/pirate/cannon.h"
#include "headers/pirate/pirate.h"

#include "headers/world_particle.h"
#include "headers/force_gen.h"
#include "headers/force_registry.h"
#include "headers/gravity_force_gen.h"
#include "headers/drag_force_gen.h"

#include "headers/shader.h"
#include "headers/render_particle.h"
#include "headers/main.h"

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
pirategame::Pirate    pirate;
GravityForceGenerator gravityGen(glm::vec3(0.0f, -GRAVITY, 0.0f));
DragForceGenerator    dragGen(DRAG_K1, DRAG_K2);

// ───────────────────────────────────────────────────────────────────────────
// Scene objects
// ───────────────────────────────────────────────────────────────────────────
pirategame::Cannon         gCannon(-350.0f, -350.0f);
vector<pirategame::Bottle> gBottles;

// ───────────────────────────────────────────────────────────────────────────
// Meshes
// ───────────────────────────────────────────────────────────────────────────
ObjMesh pirateMesh;   // reuses sphere.obj for now — swap for pirate.obj later
ObjMesh bottleMesh;   // same sphere, different color + scale
ObjMesh cannonMesh;   // same sphere for cannon base

// ───────────────────────────────────────────────────────────────────────────
// Bottles
// ───────────────────────────────────────────────────────────────────────────
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
// Level / state transitions
// ───────────────────────────────────────────────────────────────────────────
void startLevel(int level)
{
    gCurrentLevel   = level;
    gLevel          = getLevelConfig(level);
    gBottlesThisRun = 0;
    pWorld          = melonyx::PhysicsWorld();
    loadLevelBottles(level);

    pirate.reset(gCannon.muzzlePosition());
    gState = GameState::AIMING;
    cout << "[LEVEL] Level " << level << " | quota: " << gLevel.bottleQuota << "\n";
}

void launchPirate()
{
    pWorld = melonyx::PhysicsWorld();
    pirate.reset(gCannon.muzzlePosition());
    pirate.Velocity = gCannon.fire();
    pirate.onLaunch(gCannon.power);  // sets spin based on power

    pWorld.AddParticle(&pirate);
    pWorld.forceRegistry.Add(&pirate, &gravityGen);
    pWorld.forceRegistry.Add(&pirate, &dragGen);

    gState = GameState::LAUNCHED;
    cout << "[LAUNCH] angle=" << gCannon.angleDeg
         << " power=" << gCannon.power << "\n";
}

void onFlightEnd(bool drowned)
{
    gBottlesThisRun = pirategame::bottlesCollected(gBottles);
    pirate.inWater  = drowned;
    pirate.landed   = !drowned;

    cout << "[RESULT] Bottles: " << gBottlesThisRun << "\n";

    if (drowned || gBottlesThisRun == 0) {
        gState        = GameState::GAME_OVER;
        gBottlesTotal = 0;
        startLevel(1);
        cout << "[STATE] GAME_OVER\n";
        return;
    }

    gBottlesTotal += gBottlesThisRun;

    if (gCurrentLevel < 3) {
        startLevel(gCurrentLevel + 1);
    } else {
        if (gBottlesTotal >= TOTAL_QUOTA) {
            gState = GameState::WIN;
            cout << "[STATE] WIN — " << gBottlesTotal << " bottles\n";
        } else {
            gState        = GameState::GAME_OVER;
            gBottlesTotal = 0;
            startLevel(1);
            cout << "[STATE] GAME_OVER — quota missed\n";
        }
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Trajectory preview
// ───────────────────────────────────────────────────────────────────────────
vector<glm::vec2> computeTrajectory()
{
    vector<glm::vec2> pts;
    glm::vec2 pos(gCannon.muzzlePosition());
    glm::vec2 vel(gCannon.fire());

    for (int i = 0; i < PREVIEW_STEPS; i++) {
        vel.y -= GRAVITY * PREVIEW_DT;
        vel   *= (1.0f - DRAG_K1 * PREVIEW_DT);
        pos   += vel * PREVIEW_DT;
        pts.push_back(pos);
        if (pos.y < gLevel.waterLine) break;
    }
    return pts;
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
        if (gState == GameState::AIMING) {
            if (key == GLFW_KEY_LEFT)  gCannon.rotateLeft();
            if (key == GLFW_KEY_RIGHT) gCannon.rotateRight();
            if (key == GLFW_KEY_UP)    gCannon.powerUp();
            if (key == GLFW_KEY_DOWN)  gCannon.powerDown();
            if (key == GLFW_KEY_SPACE) launchPirate();
            if (key == GLFW_KEY_R)     startLevel(gCurrentLevel);
        }
        if (gState == GameState::GAME_OVER || gState == GameState::WIN) {
            if (key == GLFW_KEY_R) startLevel(1);
        }
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Dot renderer (trajectory + water line)
// ───────────────────────────────────────────────────────────────────────────
GLuint gShader = 0;
GLuint gDotVAO = 0;
GLuint gDotVBO = 0;

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
// Draw a mesh at a given model matrix
// ───────────────────────────────────────────────────────────────────────────
void drawMesh(ObjMesh& mesh, const glm::mat4& model,
              const glm::mat4& proj, const glm::mat4& view,
              float r, float g, float b)
{
    glUniformMatrix4fv(glGetUniformLocation(gShader, "model"),      1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(gShader, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gShader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3f(glGetUniformLocation(gShader, "color"), r, g, b);
    mesh.draw();
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

    if (!gladLoadGL()) {
        cerr << "GLAD failed\n"; glfwTerminate(); return -1;
    }
    cout << "OpenGL: " << glGetString(GL_VERSION) << "\n";

    gShader = compileShaders("shaders/sample.vert", "shaders/sample.frag");
    if (!gShader) { cerr << "Shader failed\n"; glfwTerminate(); return -1; }

    // Load meshes — all using sphere.obj for now
    if (!pirateMesh.load("3D/sphere.obj"))  { cerr << "pirate mesh failed\n"; return -1; }
    if (!bottleMesh.load("3D/sphere.obj"))  { cerr << "bottle mesh failed\n"; return -1; }
    if (!cannonMesh.load("3D/sphere.obj"))  { cerr << "cannon mesh failed\n"; return -1; }
    pirateMesh.upload();
    bottleMesh.upload();
    cannonMesh.upload();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
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
                pirate.updateRotation(timestep_sec);  // spin every step

                auto result = pirategame::checkCollisions(
                    pirate, gBottles, gLevel.waterLine);

                if (result.hitBottle) {
                    // Spike spin on bottle hit
                    pirate.onCollisionHit(glm::length(pirate.Velocity));
                    cout << "[HIT] " << result.bottlesHit << " bottle(s)\n";
                }

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

        glm::mat4 proj = glm::ortho(
            -ORTHO_SIZE, ORTHO_SIZE, -ORTHO_SIZE, ORTHO_SIZE, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, 500), glm::vec3(0), glm::vec3(0, 1, 0));

        // ── Cannon (yellow sphere) ────────────────────────────────────
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(gCannon.position, 0.0f));
            model = glm::rotate(model, glm::radians(gCannon.angleDeg), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(25.0f));
            drawMesh(cannonMesh, model, proj, view, 0.9f, 0.7f, 0.1f);
        }

        // ── Trajectory preview (aiming only) ─────────────────────────
        if (gState == GameState::AIMING) {
            auto traj = computeTrajectory();
            for (int i = 0; i < (int)traj.size(); i += 2)  // every other dot
                drawDot(traj[i], proj, view, 1.0f, 1.0f, 1.0f, 4.0f);
        }

        // ── Pirate (red sphere, spinning) ─────────────────────────────
        if (gState == GameState::LAUNCHED) {
            drawMesh(pirateMesh, pirate.modelMatrix(20.0f), proj, view,
                     0.9f, 0.15f, 0.15f);
        }

        // ── Bottles ───────────────────────────────────────────────────
        for (auto& b : gBottles) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(b.position, 0.0f));
            model = glm::scale(model, glm::vec3(BOTTLE_RADIUS));
            if (b.collected)
                drawMesh(bottleMesh, model, proj, view, 0.3f, 0.3f, 0.3f);
            else
                drawMesh(bottleMesh, model, proj, view, 0.2f, 0.85f, 0.3f);
        }

        // ── Water line (blue dots) ─────────────────────────────────────
        for (float x = -ORTHO_SIZE; x <= ORTHO_SIZE; x += 30.0f)
            drawDot(glm::vec2(x, gLevel.waterLine), proj, view,
                    0.1f, 0.3f, 0.9f, 5.0f);

        glfwSwapBuffers(window);
    }

    pirateMesh.cleanup();
    bottleMesh.cleanup();
    cannonMesh.cleanup();
    glDeleteVertexArrays(1, &gDotVAO);
    glDeleteBuffers(1, &gDotVBO);
    glDeleteProgram(gShader);
    glfwTerminate();
    return 0;
}