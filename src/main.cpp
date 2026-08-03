/**
 * @file main.cpp
 * @author Erica Barundia & Danie Bravo
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
#include "wind_force_gen.h"
#include "rod.h"

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
// HUD / timer / aim-guide tuning
// ───────────────────────────────────────────────────────────────────────────
constexpr int   AIM_GUIDE_STEPS = 20;
constexpr float MAX_FLIGHT_TIME = 15.0f;
constexpr float TIMER_BAR_WIDTH = 300.0f;
constexpr float TIMER_BAR_HEIGHT = 18.0f;
constexpr float TIMER_BAR_MARGIN = 24.0f;

constexpr float CANNON_MESH_FACING_OFFSET_DEG = 90.0f;

constexpr int TOTAL_LEVELS = 10;

// ── Hanging crate obstacle ──────────────────────────────────────────────────
glm::vec3 gBaseAnchorPos = glm::vec3(-180.0f, 220.0f, 0.0f);
glm::vec3 gMastAnchorPos = gBaseAnchorPos;
constexpr float CRATE_ROD_LENGTH = 80.0f;
constexpr float CRATE_RADIUS = 18.0f;
constexpr float CRATE_MASS = 4.0f;
constexpr float MAST_ANCHOR_MASS = 500.0f;
constexpr float ROPE_THICKNESS = 3.0f;

// ── Power-up Systems ────────────────────────────────────────────────────────
enum class PowerUpType { BOUNCER, BOOST_RING };

struct PowerUp {
    glm::vec3 position;
    float radius = 22.0f;
    PowerUpType type;
    bool collected = false;
};

vector<PowerUp> gPowerUps;

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
GravityForceGenerator          gravityGen(glm::vec3(0.0f, -GRAVITY, 0.0f));
melonyx::DragForceGenerator    dragGen(DRAG_K1, DRAG_K2);
melonyx::WindForceGenerator    windGen(glm::vec3(0.0f, 0.0f, 0.0f));

pirategame::Cannon          gCannon(-300.0f, 20.0f);
vector<pirategame::Bottle>  gBottles;

float gFlightElapsed = 0.0f;

melonyx::Particle gMastAnchor;
melonyx::Particle gCrate;
melonyx::Rod      gCrateRod{};

void showControls()
{
    printf("\nCONTROLS:\n");
    printf("Left/Right arrow: Increase/decrease launch power");
    printf("\nWASD: Move camera\nSpacebar: Fire cannon\nR: Restart current level\nL: Skip level (debug)\n");
}

void setLevelObstacleAnchor(int level) {
    switch (level) {
    case 1:  gBaseAnchorPos = glm::vec3(-220.0f, 250.0f, 0.0f); break;
    case 2:  gBaseAnchorPos = glm::vec3(-120.0f, 240.0f, 0.0f); break;
    case 3:  gBaseAnchorPos = glm::vec3(20.0f, 240.0f, 0.0f); break;
    case 4:  gBaseAnchorPos = glm::vec3(120.0f, 230.0f, 0.0f); break;
    case 5:  gBaseAnchorPos = glm::vec3(-50.0f, 260.0f, 0.0f); break;
    case 6:  gBaseAnchorPos = glm::vec3(80.0f, 210.0f, 0.0f); break;
    case 7:  gBaseAnchorPos = glm::vec3(-150.0f, 240.0f, 0.0f); break;
    case 8:  gBaseAnchorPos = glm::vec3(50.0f, 250.0f, 0.0f); break;
    case 9:  gBaseAnchorPos = glm::vec3(-180.0f, 220.0f, 0.0f); break;
    case 10: gBaseAnchorPos = glm::vec3(100.0f, 270.0f, 0.0f); break;
    default: gBaseAnchorPos = glm::vec3(-120.0f, 220.0f, 0.0f); break;
    }
}

void resetObstacle() {
    gMastAnchorPos = gBaseAnchorPos;
    gMastAnchor.Position = gMastAnchorPos;
    gMastAnchor.Velocity = glm::vec3(0.0f);
    gMastAnchor.mass = MAST_ANCHOR_MASS;
    gMastAnchor.radius = 6.0f;
    gMastAnchor.restitution = 0.2f;

    gCrate.Position = gMastAnchorPos + glm::vec3(0.0f, -CRATE_ROD_LENGTH, 0.0f);
    gCrate.Velocity = glm::vec3(0.0f);
    gCrate.mass = CRATE_MASS;
    gCrate.radius = CRATE_RADIUS;
    gCrate.restitution = 0.3f;

    gCrateRod.particles[0] = &gMastAnchor;
    gCrateRod.particles[1] = &gCrate;
    gCrateRod.length = CRATE_ROD_LENGTH;
    gCrateRod.restitution = 0.2f;
}

void updateMovingObstacles(float appTime, int level) {
    if (level == 1) return;

    if (level % 3 == 0) {
        gMastAnchor.Position.y = gBaseAnchorPos.y + sinf(appTime * 2.5f) * 40.0f;
        gMastAnchor.Position.x = gBaseAnchorPos.x + cosf(appTime * 1.2f) * 30.0f;
    }
    else if (level % 2 == 0) {
        gMastAnchor.Position.y = gBaseAnchorPos.y + sinf(appTime * 2.0f) * 35.0f;
    }
    else {
        gMastAnchor.Position.x = gBaseAnchorPos.x + cosf(appTime * 1.8f) * 60.0f;
    }
}

void checkPowerUpCollisions(pirategame::Pirate& p) {
    for (auto& pu : gPowerUps) {
        if (pu.collected) continue;

        float dist = glm::length(p.Position - pu.position);
        if (dist < (p.radius + pu.radius)) {
            pu.collected = true;

            if (pu.type == PowerUpType::BOUNCER) {
                p.Velocity.y = fabsf(p.Velocity.y) * 0.85f + 260.0f;
                p.Velocity.x = p.Velocity.x * 1.15f;
                cout << "[POWERUP] Bouncer Hit! Launched Skyward!\n";
            }
            else if (pu.type == PowerUpType::BOOST_RING) {
                glm::vec3 dir = glm::normalize(p.Velocity);
                p.Velocity += dir * 220.0f;
                cout << "[POWERUP] Boost Ring Passed! Accelerated Flight!\n";
            }
        }
    }
}

// Forward Declarations
void startLevel(int level);
void onFlightEnd(bool drowned);

// ───────────────────────────────────────────────────────────────────────────
// Dynamic Trajectory Generation for Preview
// ───────────────────────────────────────────────────────────────────────────
vector<glm::vec2> simulateArc(float angleDeg, float power, float waterLine) {
    vector<glm::vec2> pts;
    glm::vec2 pos = gCannon.muzzlePosition();

    float rad = glm::radians(angleDeg);
    glm::vec2 vel(cosf(rad) * power, sinf(rad) * power);

    for (int i = 0; i < 500; i++) {
        vel.y -= GRAVITY * PREVIEW_DT;
        vel.x += gLevel.windStrength * PREVIEW_DT;   // matches WindForceGenerator
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

GLuint gCubeVAO = 0, gCubeVBO = 0;

GLuint gUIShaderProgram = 0;
GLuint gUIQuadVAO = 0, gUIQuadVBO = 0;

void initGeometries() {
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

    glGenVertexArrays(1, &gDotVAO);
    glGenBuffers(1, &gDotVBO);
    glBindVertexArray(gDotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gDotVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

    float uiQuadVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
    };
    glGenVertexArrays(1, &gUIQuadVAO);
    glGenBuffers(1, &gUIQuadVBO);
    glBindVertexArray(gUIQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gUIQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiQuadVertices), uiQuadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

GLuint createUIShader() {
    const char* vertCode = R"(
        #version 460 core
        layout (location = 0) in vec2 aPos;

        uniform mat4 uiProjection;
        uniform vec4 rect;

        void main() {
            vec2 pixelPos = rect.xy + aPos * rect.zw;
            gl_Position = uiProjection * vec4(pixelPos, 0.0, 1.0);
        }
    )";

    const char* fragCode = R"(
        #version 460 core
        out vec4 FragColor;
        uniform vec4 color;

        void main() {
            FragColor = color;
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

void drawUIRect(float x, float y, float w, float h, glm::vec4 color) {
    glUseProgram(gUIShaderProgram);

    glm::mat4 uiProjection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(gUIShaderProgram, "uiProjection"), 1, GL_FALSE, glm::value_ptr(uiProjection));
    glUniform4f(glGetUniformLocation(gUIShaderProgram, "rect"), x, y, w, h);
    glUniform4f(glGetUniformLocation(gUIShaderProgram, "color"), color.r, color.g, color.b, color.a);

    glBindVertexArray(gUIQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ───────────────────────────────────────────────────────────────────────────
// Level Setup
// ───────────────────────────────────────────────────────────────────────────
void loadLevelBottles(int level) {
    gBottles.clear();
    gPowerUps.clear();

    switch (level) {
    case 1:
        gLevel.bottleQuota = 3;
        BottlePlacer::placeBottle(-100.0f, 110.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(0.0f, 85.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(100.0f, 60.0f, 0.0f, gBottles);
        break;

    case 2:
        gLevel.bottleQuota = 3;
        BottlePlacer::placeBottle(-150.0f, 100.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(-50.0f, 130.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(50.0f, 30.0f, 0.0f), 28.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(100.0f, 140.0f, 0.0f, gBottles);
        break;

    case 3:
        gLevel.bottleQuota = 3;
        BottlePlacer::placeBottle(-160.0f, 130.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(-60.0f, 180.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(50.0f, 160.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(150.0f, 110.0f, 0.0f, gBottles);
        break;

    case 4:
        gLevel.bottleQuota = 4;
        BottlePlacer::placeBottle(-160.0f, 140.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(-30.0f, 220.0f, 0.0f), 20.0f, PowerUpType::BOOST_RING });
        BottlePlacer::placeBottle(80.0f, 240.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(200.0f, 210.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(300.0f, 150.0f, 0.0f, gBottles);
        break;

    case 5:
        gLevel.bottleQuota = 4;
        BottlePlacer::placeBottle(-180.0f, 120.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(-60.0f, 100.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(60.0f, 60.0f, 0.0f), 22.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(180.0f, 250.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(280.0f, 350.0f, 0.0f, gBottles);
        break;

    case 6:
        gLevel.bottleQuota = 5;
        BottlePlacer::placeBottle(-180.0f, 130.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(-80.0f, 190.0f, 0.0f), 20.0f, PowerUpType::BOOST_RING });
        BottlePlacer::placeBottle(20.0f, 220.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(140.0f, 190.0f, 0.0f), 20.0f, PowerUpType::BOOST_RING });
        BottlePlacer::placeBottle(240.0f, 180.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(340.0f, 140.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(420.0f, 90.0f, 0.0f, gBottles);
        break;

    case 7:
        gLevel.bottleQuota = 4;
        BottlePlacer::placeBottle(-190.0f, 150.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(-90.0f, 230.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(10.0f, 60.0f, 0.0f), 24.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(120.0f, 420.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(220.0f, 450.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(360.0f, 380.0f, 0.0f, gBottles);
        break;

    case 8:
        gLevel.bottleQuota = 6;
        BottlePlacer::placeBottle(-200.0f, 140.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(-100.0f, 220.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(0.0f, 240.0f, 0.0f), 20.0f, PowerUpType::BOOST_RING });
        BottlePlacer::placeBottle(100.0f, 220.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(190.0f, 180.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(270.0f, 130.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(340.0f, 80.0f, 0.0f, gBottles);
        break;

    case 9:
        gLevel.bottleQuota = 4;
        BottlePlacer::placeBottle(-180.0f, 150.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(-70.0f, 50.0f, 0.0f), 22.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(30.0f, 220.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(120.0f, 200.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(210.0f, 60.0f, 0.0f), 22.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(290.0f, 210.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(370.0f, 170.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(440.0f, 110.0f, 0.0f, gBottles);
        break;

    case 10:
        gLevel.bottleQuota = 3;
        BottlePlacer::placeBottle(-200.0f, 150.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(-100.0f, 50.0f, 0.0f), 22.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(0.0f, 240.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(100.0f, 250.0f, 0.0f), 20.0f, PowerUpType::BOOST_RING });
        BottlePlacer::placeBottle(190.0f, 220.0f, 0.0f, gBottles);
        gPowerUps.push_back({ glm::vec3(300.0f, 60.0f, 0.0f), 22.0f, PowerUpType::BOUNCER });
        BottlePlacer::placeBottle(360.0f, 200.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(430.0f, 150.0f, 0.0f, gBottles);
        BottlePlacer::placeBottle(490.0f, 300.0f, 0.0f, gBottles);
        break;
    }
}

void startLevel(int level) {
    gCurrentLevel = level;
    gLevel = getLevelConfig(level);
    gBottlesThisRun = 0;
    pWorld = melonyx::PhysicsWorld();

    setLevelObstacleAnchor(level);
    loadLevelBottles(level);
    resetObstacle();

    pirate.reset(gCannon.muzzlePosition());
    gState = GameState::AIMING;
    cout << "\n=========================================\n";
    cout << "[3D LEVEL] Level " << level << " / " << TOTAL_LEVELS << " | Quota: " << gLevel.bottleQuota << "\n";
    cout << "=========================================\n";
}

void launchPirate() {
    pWorld = melonyx::PhysicsWorld();
    pirate.reset(gCannon.muzzlePosition());
    pirate.Velocity = gCannon.fire();
    pirate.onLaunch(gCannon.power);

    pWorld.AddParticle(&pirate);
    pWorld.forceRegistry.Add(&pirate, &gravityGen);

    windGen.SetForce(glm::vec3(gLevel.windStrength, 0.0f, 0.0f));
    pWorld.forceRegistry.Add(&pirate, &windGen);

    pWorld.Particles.push_back(&gMastAnchor);
    pWorld.AddParticle(&gCrate);
    pWorld.Links.push_back(&gCrateRod);

    gFlightElapsed = 0.0f;
    gState = GameState::LAUNCHED;
}

void onFlightEnd(bool drowned) {
    gBottlesThisRun = pirategame::bottlesCollected(gBottles);
    pirate.inWater = drowned;
    pirate.landed = !drowned;

    cout << "[TIMER] Flight time: " << gFlightElapsed << "s\n";

    if (drowned || gBottlesThisRun < gLevel.bottleQuota) {
        gState = GameState::GAME_OVER;
        cout << "[STATE] GAME OVER! Quota missed. Press R to retry Level " << gCurrentLevel << ".\n";
        return;
    }

    gBottlesTotal += gBottlesThisRun;

    if (gCurrentLevel < TOTAL_LEVELS) {
        cout << "[STATE] Level " << gCurrentLevel << " Cleared! Advancing...\n";
        startLevel(gCurrentLevel + 1);
    }
    else {
        gState = GameState::WIN;
        cout << "[STATE] VICTORY! All levels cleared with " << gBottlesTotal << " total bottles!\n";
    }
}

vector<glm::vec2> computeTrajectory() {
    auto full = simulateArc(gCannon.angleDeg, gCannon.power, gLevel.waterLine);
    if ((int)full.size() > AIM_GUIDE_STEPS) {
        full.resize(AIM_GUIDE_STEPS);
    }
    return full;
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) cameraPos.z -= 20.0f;
        if (key == GLFW_KEY_S) cameraPos.z += 20.0f;
        if (key == GLFW_KEY_A) { cameraPos.x -= 20.0f; cameraTarget.x -= 20.0f; }
        if (key == GLFW_KEY_D) { cameraPos.x += 20.0f; cameraTarget.x += 20.0f; }

        if (key == GLFW_KEY_L && action == GLFW_PRESS) {
            if (gCurrentLevel < TOTAL_LEVELS) {
                cout << "[SKIP] Skipping Level " << gCurrentLevel << "...\n";
                startLevel(gCurrentLevel + 1);
            }
            else {
                gState = GameState::WIN;
            }
            return;
        }

        if (gState == GameState::AIMING) {
            if (key == GLFW_KEY_LEFT)  gCannon.rotateLeft();
            if (key == GLFW_KEY_RIGHT) gCannon.rotateRight();
            if (key == GLFW_KEY_UP)    gCannon.powerUp();
            if (key == GLFW_KEY_DOWN)  gCannon.powerDown();
            if (key == GLFW_KEY_SPACE) launchPirate();
            if (key == GLFW_KEY_R)     startLevel(gCurrentLevel);
        }
        else if (gState == GameState::GAME_OVER || gState == GameState::WIN) {
            if (key == GLFW_KEY_R) startLevel(gCurrentLevel); // <--- Restarts CURRENT level on Game Over!
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

void drawTimerBar() {
    if (gState != GameState::LAUNCHED) return;

    float remaining = glm::clamp(1.0f - (gFlightElapsed / MAX_FLIGHT_TIME), 0.0f, 1.0f);

    float x = TIMER_BAR_MARGIN;
    float y = (float)WINDOW_HEIGHT - TIMER_BAR_MARGIN - TIMER_BAR_HEIGHT;

    drawUIRect(x, y, TIMER_BAR_WIDTH, TIMER_BAR_HEIGHT, glm::vec4(0.0f, 0.0f, 0.0f, 0.4f));

    glm::vec3 fillColor = glm::mix(glm::vec3(0.85f, 0.15f, 0.15f), glm::vec3(0.2f, 0.85f, 0.3f), remaining);
    drawUIRect(x, y, TIMER_BAR_WIDTH * remaining, TIMER_BAR_HEIGHT, glm::vec4(fillColor, 0.9f));
}

void drawTextYOUWIN(float centerX, float centerY, float scale, glm::vec4 color) {
    static const int chars[6][5] = {
        {0b10001, 0b10001, 0b01010, 0b00100, 0b00100}, // Y
        {0b01110, 0b10001, 0b10001, 0b10001, 0b01110}, // O
        {0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // U
        {0b10001, 0b10001, 0b10101, 0b10101, 0b01010}, // W
        {0b01110, 0b00100, 0b00100, 0b00100, 0b01110}, // I
        {0b10001, 0b11001, 0b10101, 0b10011, 0b10001}  // N
    };

    float charWidth = 5 * scale;
    float spacing = 2 * scale;
    float wordSpacing = 6 * scale;

    float totalWidth = (6 * charWidth) + (4 * spacing) + wordSpacing;
    float startX = centerX - totalWidth * 0.5f;

    for (int c = 0; c < 6; c++) {
        float xOffset = startX + c * (charWidth + spacing) + (c >= 3 ? wordSpacing : 0.0f);

        for (int r = 0; r < 5; r++) {
            int rowBits = chars[c][r];
            for (int col = 0; col < 5; col++) {
                if (rowBits & (1 << (4 - col))) {
                    drawUIRect(xOffset + col * scale, centerY + (4 - r) * scale, scale, scale, color);
                }
            }
        }
    }
}

void drawWinScreen(float elapsedSeconds) {
    glDisable(GL_DEPTH_TEST);

    drawUIRect(0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, glm::vec4(0.05f, 0.25f, 0.08f, 0.55f));

    float pulse = 0.5f + 0.5f * sinf(elapsedSeconds * 3.0f);
    float bannerH = 160.0f;
    float bannerY = WINDOW_HEIGHT * 0.5f - bannerH * 0.5f;
    drawUIRect(0.0f, bannerY, (float)WINDOW_WIDTH, bannerH, glm::vec4(0.85f, 0.65f, 0.15f, 0.35f + 0.25f * pulse));

    drawTextYOUWIN(WINDOW_WIDTH * 0.5f, bannerY + bannerH * 0.45f, 8.0f, glm::vec4(1.0f, 0.95f, 0.3f, 1.0f));

    float dotSize = 22.0f;
    float spacing = 10.0f;
    float totalW = gBottlesTotal * (dotSize + spacing) - spacing;
    float startX = WINDOW_WIDTH * 0.5f - totalW * 0.5f;
    float dotY = bannerY - dotSize - 20.0f;
    for (int i = 0; i < gBottlesTotal; i++) {
        drawUIRect(startX + i * (dotSize + spacing), dotY, dotSize, dotSize, glm::vec4(0.9f, 0.75f, 0.2f, 0.95f));
    }

    glEnable(GL_DEPTH_TEST);
}

void drawBottleQuota()
{
    constexpr float size = 20.0f;
    constexpr float spacing = 8.0f;

    float x = 20.0f;
    float y = WINDOW_HEIGHT - 40.0f;

    int collected = pirategame::bottlesCollected(gBottles);

    for (int i = 0; i < gLevel.bottleQuota; i++)
    {
        glm::vec4 color =
            (i < collected)
            ? glm::vec4(0.15f, 0.9f, 0.2f, 1.0f)   // collected
            : glm::vec4(0.35f, 0.35f, 0.35f, 1.0f); // remaining

        drawUIRect(
            x + i * (size + spacing),
            y,
            size,
            size,
            color
        );
    }
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
    gUIShaderProgram = createUIShader();
    initGeometries();

    gPirateLoaded = gPirateMesh.load("3D/pirate.obj", "3D/");
    gBottleLoaded = gBottleMesh.load("3D/beer.obj", "3D/");
    gCannonLoaded = gCannonMesh.load("3D/cannon.obj", "3D/");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    showControls();

    startLevel(1);

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    float gAppTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        curr_time = clock::now();
        auto dur = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
        curr_ns += dur;
        gAppTime += chrono::duration<float>(dur).count();

        updateMovingObstacles(gAppTime, gCurrentLevel);
        gCrate.Position = gMastAnchor.Position + glm::vec3(0.0f, -CRATE_ROD_LENGTH, 0.0f);

        while (curr_ns >= timestep) {
            curr_ns -= timestep;

            if (gState == GameState::LAUNCHED) {
                pWorld.Update(timestep_sec); // velocity handled in particlecontact.cpp
                pirate.updateRotation(timestep_sec);
                gFlightElapsed += timestep_sec;

                checkPowerUpCollisions(pirate);

                auto result = pirategame::checkCollisions(pirate, gBottles, gLevel.waterLine);

                if (result.hitBottle) {
                    pirate.onCollisionHit(glm::length(pirate.Velocity));
                }

                float distToCrate = glm::length(pirate.Position - gCrate.Position);
                float distToMast = glm::length(pirate.Position - gMastAnchor.Position);
                if (distToCrate < pirate.radius + gCrate.radius ||
                    distToMast < pirate.radius + gMastAnchor.radius) {
                    pirate.onCollisionHit(glm::length(pirate.Velocity));
                }

                // Win
                int currentHitCount = pirategame::bottlesCollected(gBottles);
                if (currentHitCount >= gLevel.bottleQuota) {
                    onFlightEnd(false);
                    break;
                }

                // Lose/Fail Condition
                bool outOfBounds = pirate.Position.x > ORTHO_SIZE + 50.0f
                    || pirate.Position.x < -ORTHO_SIZE - 50.0f;

                if (result.hitWater || outOfBounds) {
                    onFlightEnd(result.hitWater);
                    break;
                }

                if (gFlightElapsed >= MAX_FLIGHT_TIME) {
                    cout << "[TIMER] Time's up -- shot timed out.\n";
                    onFlightEnd(true);
                    break;
                }
            }
        }

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
            barrelModel = glm::rotate(barrelModel, glm::radians(gCannon.angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
            barrelModel = glm::rotate(barrelModel, glm::radians(CANNON_MESH_FACING_OFFSET_DEG), glm::vec3(0.0f, 1.0f, 0.0f));
            barrelModel = glm::scale(barrelModel, glm::vec3(8.0f));

            if (gCannonLoaded) {
                gCannonMesh.draw(gShaderProgram, barrelModel, proj, view, glm::vec3(0.85f, 0.65f, 0.20f));
            }
            else {
                drawFallbackCube(barrelModel, proj, view, glm::vec3(0.90f, 0.70f, 0.15f));
            }
        }

        // 2b. Hanging crate obstacle
        {
            glm::vec3 anchorPos = gMastAnchor.Position;
            glm::vec3 cratePos = gCrate.Position;

            glm::mat4 mastModel = glm::translate(glm::mat4(1.0f), anchorPos);
            mastModel = glm::scale(mastModel, glm::vec3(10.0f));
            drawFallbackCube(mastModel, proj, view, glm::vec3(0.35f, 0.25f, 0.15f));

            glm::vec3 diff = cratePos - anchorPos;
            float     ropeLen = glm::length(diff);
            if (ropeLen > 0.001f) {
                glm::vec3 dir = diff / ropeLen;
                glm::vec3 up(0.0f, 1.0f, 0.0f);
                glm::vec3 axis = glm::cross(up, dir);
                float     angle = acosf(glm::clamp(glm::dot(up, dir), -1.0f, 1.0f));

                glm::mat4 ropeModel = glm::translate(glm::mat4(1.0f), anchorPos + diff * 0.5f);
                if (glm::length(axis) > 1e-5f) {
                    ropeModel = glm::rotate(ropeModel, angle, glm::normalize(axis));
                }
                else if (angle > 3.0f) {
                    ropeModel = glm::rotate(ropeModel, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                }
                ropeModel = glm::scale(ropeModel, glm::vec3(ROPE_THICKNESS, ropeLen, ROPE_THICKNESS));
                drawFallbackCube(ropeModel, proj, view, glm::vec3(0.55f, 0.45f, 0.30f));
            }

            glm::mat4 crateModel = glm::translate(glm::mat4(1.0f), cratePos);
            crateModel = glm::scale(crateModel, glm::vec3(gCrate.radius));
            drawFallbackCube(crateModel, proj, view, glm::vec3(0.60f, 0.40f, 0.18f));
        }

        // 2c. Render Power-Up Objects
        for (const auto& pu : gPowerUps) {
            if (pu.collected) continue;

            glm::mat4 puModel = glm::translate(glm::mat4(1.0f), pu.position);
            puModel = glm::scale(puModel, glm::vec3(pu.radius));

            glm::vec3 color = (pu.type == PowerUpType::BOUNCER)
                ? glm::vec3(0.95f, 0.80f, 0.10f)
                : glm::vec3(0.10f, 0.85f, 0.95f);

            drawFallbackCube(puModel, proj, view, color);
        }

        // 3. Aiming Guide Trajectory
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

        // 5. Target Bottles
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

        // 6. HUD Overlay

        drawBottleQuota();
        //drawTimerBar();

        if (gState == GameState::WIN)
        {
            drawWinScreen(gAppTime);
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