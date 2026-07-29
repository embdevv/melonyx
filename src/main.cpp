/**
 * @file main.cpp
 * @author Erica Mauriz Barundia & Danie Bravo
 *
 * ARR You Drunk Yet? (3D Physics Visualizer with ObjMesh)
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

#include "pirate/constants.h"
#include "pirate/game_state.h"
#include "pirate/level.h"
#include "pirate/bottle.h"
#include "pirate/collision.h"
#include "pirate/cannon.h"
#include "pirate/pirate.h"

#include "world_particle.h"
#include "force_gen.h"
#include "force_registry.h"
#include "gravity_force_gen.h"
#include "drag_force_gen.h"

#include "OpenGLObject.h"
#include "pirate/bottle_placer.h"

using namespace std;
using namespace chrono_literals;

// ───────────────────────────────────────────────────────────────────────────
// Window & 3D Camera Setup
// ───────────────────────────────────────────────────────────────────────────
const int   WINDOW_WIDTH = 1280;
const int   WINDOW_HEIGHT = 720;
const string WINDOW_TITLE = "ARR You Drunk Yet? (3D Physics Visualizer)";

glm::vec3 cameraPos = glm::vec3(0.0f, 80.0f, 675.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, -20.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// ───────────────────────────────────────────────────────────────────────────
// Game State & Physics
// ───────────────────────────────────────────────────────────────────────────
GameState   gState = GameState::AIMING;
int         gCurrentLevel = 1;
int         gBottlesTotal = 0;
int         gBottlesThisRun = 0;
LevelConfig gLevel = getLevelConfig(1);

melonyx::PhysicsWorld pWorld;
pirategame::Pirate    pirate;
GravityForceGenerator gravityGen(glm::vec3(0.0f, -GRAVITY, 0.0f));
DragForceGenerator    dragGen(DRAG_K1, DRAG_K2);

pirategame::Cannon          gCannon(-300.0f, 20.0f);
vector<pirategame::Bottle> gBottles;

// Forward Declarations
void startLevel(int level);
void onFlightEnd(bool drowned);

// ───────────────────────────────────────────────────────────────────────────
// Dynamic Trajectory Generation for Sensible Bottle Layouts
// ───────────────────────────────────────────────────────────────────────────
vector<glm::vec2> simulateArc(float angleDeg, float power, float waterLine) {
    vector<glm::vec2> pts;
    glm::vec2 pos = gCannon.muzzlePosition();

    // Convert angle and power to initial velocity vector
    float rad = glm::radians(angleDeg);
    glm::vec2 vel(cos(rad) * power, sin(rad) * power);

    for (int i = 0; i < 500; i++) {
        vel.y -= GRAVITY * PREVIEW_DT;
        vel *= (1.0f - DRAG_K1 * PREVIEW_DT);
        pos += vel * PREVIEW_DT;
        pts.push_back(pos);
        if (pos.y < waterLine) break;
    }
    return pts;
}

// ───────────────────────────────────────────────────────────────────────────
// 3D Objects & Models
// ───────────────────────────────────────────────────────────────────────────
GLuint gShaderProgram = 0;
GLuint gPlaneVAO = 0, gPlaneVBO = 0;
GLuint gDotVAO = 0, gDotVBO = 0;

ObjMesh gPirateMesh;
ObjMesh gBottleMesh;
ObjMesh gCannonMesh;

bool gPirateLoaded = false;
bool gBottleLoaded = false;
bool gCannonLoaded = false;

// Fallback Cube Mesh
GLuint gCubeVAO = 0, gCubeVBO = 0;

void initGeometries() {
    // 1. Water Surface
    float planeVertices[] = {
        -2000.0f, 0.0f, -1000.0f,   0.0f, 1.0f, 0.0f,
         2000.0f, 0.0f, -1000.0f,   0.0f, 1.0f, 0.0f,
         2000.0f, 0.0f,  1000.0f,   0.0f, 1.0f, 0.0f,
         2000.0f, 0.0f,  1000.0f,   0.0f, 1.0f, 0.0f,
        -2000.0f, 0.0f,  1000.0f,   0.0f, 1.0f, 0.0f,
        -2000.0f, 0.0f, -1000.0f,   0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &gPlaneVAO);
    glGenBuffers(1, &gPlaneVBO);
    glBindVertexArray(gPlaneVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 2. Trajectory Dots
    glGenVertexArrays(1, &gDotVAO);
    glGenBuffers(1, &gDotVBO);
    glBindVertexArray(gDotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gDotVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 3. Fallback Cube
    float cubeVertices[] = {
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f
    };
    glGenVertexArrays(1, &gCubeVAO);
    glGenBuffers(1, &gCubeVBO);
    glBindVertexArray(gCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

GLuint create3DLightingShader() {
    const char* vertCode = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 FragPos;
        out vec3 FragNormal;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            FragNormal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char* fragCode = R"(
        #version 460 core
        in vec3 FragPos;
        in vec3 FragNormal;
        out vec4 FragColor;

        uniform vec3 color;
        uniform vec3 lightPos;

        void main() {
            float ambientStrength = 0.35;
            vec3 ambient = ambientStrength * color;

            vec3 norm = normalize(FragNormal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * color;

            FragColor = vec4(ambient + diffuse, 1.0);
        }
    )";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertCode, NULL);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragCode, NULL);
    glCompileShader(fShader);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vShader);
    glAttachShader(prog, fShader);
    glLinkProgram(prog);

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return prog;
}

// ───────────────────────────────────────────────────────────────────────────
// Level Setup: Trajectory-Based Layouts (100% Achievable)
// ───────────────────────────────────────────────────────────────────────────
void loadLevelBottles(int level) {
    gBottles.clear();

    if (level == 1) {
        // Level 1: Low-angle, short-range 3-bottle sweep
        // Target Solution Angle: 30 degrees, Power: 350
        gLevel.bottleQuota = 3;
        auto arc = simulateArc(30.0f, 250.0f, gLevel.waterLine);

        if (arc.size() >= 30) {
            // Pick 3 evenly spaced points along the flight trajectory
            size_t step = arc.size() / 4;
            BottlePlacer::placeBottle(arc[step].x, arc[step].y, 0.0f, gBottles);
            BottlePlacer::placeBottle(arc[step * 2].x, arc[step * 2].y, 0.0f, gBottles);
            BottlePlacer::placeBottle(arc[step * 3].x, arc[step * 3].y, 0.0f, gBottles);
        }
    }
    else if (level == 2) {
        // Level 2: Mid-angle, medium-range 4-bottle sweep
        // Target Solution Angle: 45 degrees, Power: 450
        gLevel.bottleQuota = 4;
        auto arc = simulateArc(45.0f, 450.0f, gLevel.waterLine);

        if (arc.size() >= 40) {
            size_t step = arc.size() / 5;
            for (int i = 1; i <= 4; i++) {
                BottlePlacer::placeBottle(arc[step * i].x, arc[step * i].y, 0.0f, gBottles);
            }
        }
    }
    else if (level == 3) {
        // Level 3: High lobbing arc, long-range 5-bottle sweep
        // Target Solution Angle: 60 degrees, Power: 550
        gLevel.bottleQuota = 5;
        auto arc = simulateArc(60.0f, 300.0f, gLevel.waterLine);

        if (arc.size() >= 50) {
            size_t step = arc.size() / 6;
            for (int i = 1; i <= 5; i++) {
                BottlePlacer::placeBottle(arc[step * i].x, arc[step * i].y, 0.0f, gBottles);
            }
        }
    }
}

void startLevel(int level) {
    gCurrentLevel = level;
    gLevel = getLevelConfig(level);
    gBottlesThisRun = 0;
    pWorld = melonyx::PhysicsWorld();

    loadLevelBottles(level);

    pirate.reset(gCannon.muzzlePosition());
    gState = GameState::AIMING;
    cout << "\n=========================================\n";
    cout << "[3D LEVEL] Level " << level << " | Quota: " << gLevel.bottleQuota << "\n";
    cout << "=========================================\n";
}

void launchPirate() {
    pWorld = melonyx::PhysicsWorld();
    pirate.reset(gCannon.muzzlePosition());
    pirate.Velocity = gCannon.fire();
    pirate.onLaunch(gCannon.power);

    pWorld.AddParticle(&pirate);
    pWorld.forceRegistry.Add(&pirate, &gravityGen);
    pWorld.forceRegistry.Add(&pirate, &dragGen);

    gState = GameState::LAUNCHED;
}

void onFlightEnd(bool drowned) {
    gBottlesThisRun = pirategame::bottlesCollected(gBottles);
    pirate.inWater = drowned;
    pirate.landed = !drowned;

    if (drowned || gBottlesThisRun < gLevel.bottleQuota) {
        gState = GameState::GAME_OVER;
        gBottlesTotal = 0;
        cout << "[STATE] GAME OVER! Quota missed. Press R to restart.\n";
        return;
    }

    gBottlesTotal += gBottlesThisRun;

    if (gCurrentLevel < 3) {
        cout << "[STATE] Level " << gCurrentLevel << " Cleared! Advancing...\n";
        startLevel(gCurrentLevel + 1);
    }
    else {
        gState = GameState::WIN;
        cout << "[STATE] VICTORY! All levels cleared with " << gBottlesTotal << " total bottles!\n";
    }
}

vector<glm::vec2> computeTrajectory() {
    return simulateArc(gCannon.angleDeg, gCannon.power, gLevel.waterLine);
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Camera controls
        if (key == GLFW_KEY_W) cameraPos.z -= 20.0f;
        if (key == GLFW_KEY_S) cameraPos.z += 20.0f;
        if (key == GLFW_KEY_A) { cameraPos.x -= 20.0f; cameraTarget.x -= 20.0f; }
        if (key == GLFW_KEY_D) { cameraPos.x += 20.0f; cameraTarget.x += 20.0f; }

        if (gState == GameState::AIMING) {
            if (key == GLFW_KEY_LEFT)  gCannon.rotateLeft();
            if (key == GLFW_KEY_RIGHT) gCannon.rotateRight();
            if (key == GLFW_KEY_UP)    gCannon.powerUp();
            if (key == GLFW_KEY_DOWN)  gCannon.powerDown();
            if (key == GLFW_KEY_SPACE) launchPirate();
            if (key == GLFW_KEY_R)     startLevel(gCurrentLevel);
        }
        else if (gState == GameState::GAME_OVER || gState == GameState::WIN) {
            if (key == GLFW_KEY_R) startLevel(1);
        }
    }
}

void drawDot(const glm::vec3& pos, const glm::mat4& proj, const glm::mat4& view, float r, float g, float b, float size = 8.0f) {
    float v[3] = { pos.x, pos.y, pos.z };
    glBindBuffer(GL_ARRAY_BUFFER, gDotVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3f(glGetUniformLocation(gShaderProgram, "color"), r, g, b);

    glPointSize(size);
    glBindVertexArray(gDotVAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
}

void drawFallbackCube(const glm::mat4& model, const glm::mat4& proj, const glm::mat4& view, const glm::vec3& color) {
    glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3f(glGetUniformLocation(gShaderProgram, "color"), color.r, color.g, color.b);

    glBindVertexArray(gCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

int main() {
    constexpr chrono::nanoseconds timestep(16ms);
    constexpr float timestep_sec = timestep.count() / (float)(1E09);

    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { glfwTerminate(); return -1; }

    gShaderProgram = create3DLightingShader();
    initGeometries();

    // Load Models
    gPirateLoaded = gPirateMesh.load("3D/pirate.obj", "3D/");
    gBottleLoaded = gBottleMesh.load("3D/beer.obj", "3D/");
    gCannonLoaded = gCannonMesh.load("3D/cannon.obj", "3D/");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    startLevel(1);

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        curr_time = clock::now();
        auto dur = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
        curr_ns += dur;

        while (curr_ns >= timestep) {
            curr_ns -= timestep;

            if (gState == GameState::LAUNCHED) {
                pWorld.Update(timestep_sec);
                pirate.updateRotation(timestep_sec);

                auto result = pirategame::checkCollisions(pirate, gBottles, gLevel.waterLine);

                if (result.hitBottle) {
                    pirate.onCollisionHit(glm::length(pirate.Velocity));
                }

                // Instant next level check once quota is reached mid-flight
                int currentHitCount = pirategame::bottlesCollected(gBottles);
                if (currentHitCount >= gLevel.bottleQuota) {
                    onFlightEnd(false /* drowned = false */);
                    break;
                }

                bool outOfBounds = pirate.Position.x > ORTHO_SIZE + 50.0f
                    || pirate.Position.x < -ORTHO_SIZE - 50.0f;

                if (result.hitWater || outOfBounds) {
                    onFlightEnd(result.hitWater);
                    break;
                }
            }
        }

        // Render Loop
        switch (gState) {
        case GameState::AIMING:    glClearColor(0.08f, 0.12f, 0.20f, 1.0f); break;
        case GameState::LAUNCHED:  glClearColor(0.10f, 0.15f, 0.25f, 1.0f); break;
        case GameState::GAME_OVER: glClearColor(0.25f, 0.05f, 0.05f, 1.0f); break;
        case GameState::WIN:       glClearColor(0.05f, 0.25f, 0.08f, 1.0f); break;
        default: break;
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(gShaderProgram);

        glUniform3f(glGetUniformLocation(gShaderProgram, "lightPos"), -150.0f, 300.0f, 200.0f);

        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 3000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        // 1. Water Plane
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, gLevel.waterLine, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(gShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniform3f(glGetUniformLocation(gShaderProgram, "color"), 0.12f, 0.42f, 0.85f);
            glBindVertexArray(gPlaneVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // 2. Cannon Mesh
        {
            glm::mat4 barrelModel = glm::mat4(1.0f);
            barrelModel = glm::translate(barrelModel, glm::vec3(gCannon.position, 0.0f));
            barrelModel = glm::rotate(barrelModel, glm::radians(gCannon.angleDeg), glm::vec3(0, 0, 1));
            barrelModel = glm::scale(barrelModel, glm::vec3(8.0f));

            if (gCannonLoaded) {
                gCannonMesh.draw(gShaderProgram, barrelModel, proj, view, glm::vec3(0.85f, 0.65f, 0.20f));
            }
            else {
                drawFallbackCube(barrelModel, proj, view, glm::vec3(0.90f, 0.70f, 0.15f));
            }
        }

        // 3. Trajectory Dots
        if (gState == GameState::AIMING) {
            auto traj = computeTrajectory();
            for (size_t i = 0; i < traj.size(); i += 2) {
                drawDot(glm::vec3(traj[i].x, traj[i].y, 0.0f), proj, view, 1.0f, 1.0f, 1.0f, 5.0f);
            }
        }

        // 4. Pirate Mesh
        if (gState == GameState::LAUNCHED || gState == GameState::AIMING) {
            glm::mat4 pModel;
            if (gState == GameState::AIMING) {
                glm::vec2 mPos = gCannon.muzzlePosition();
                pModel = glm::translate(glm::mat4(1.0f), glm::vec3(mPos, 0.0f));
                pModel = glm::scale(pModel, glm::vec3(25.0f));
            }
            else {
                pModel = pirate.modelMatrix(28.0f);
            }

            if (gPirateLoaded) {
                gPirateMesh.draw(gShaderProgram, pModel, proj, view, glm::vec3(0.9f, 0.2f, 0.2f));
            }
            else {
                drawFallbackCube(pModel, proj, view, glm::vec3(0.9f, 0.15f, 0.15f));
            }
        }

        // 5. Target Bottle Meshes
        for (const auto& b : gBottles) {
            glm::mat4 bModel = glm::mat4(1.0f);
            bModel = glm::translate(bModel, glm::vec3(b.position, 0.0f));
            bModel = glm::scale(bModel, glm::vec3(2.5f));

            glm::vec3 bColor = b.collected ? glm::vec3(0.3f) : glm::vec3(0.1f, 0.85f, 0.35f);

            if (gBottleLoaded) {
                gBottleMesh.draw(gShaderProgram, bModel, proj, view, bColor);
            }
            else {
                drawFallbackCube(bModel, proj, view, bColor);
            }
        }

        glfwSwapBuffers(window);
    }

    gPirateMesh.cleanup();
    gBottleMesh.cleanup();
    gCannonMesh.cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}