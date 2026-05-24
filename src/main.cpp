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

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headers/main.h"
#include "headers/shader.h"

#include <chrono>

using namespace std;
using namespace chrono_literals;


// ===== WINDOW =====
// Square window
const int    WINDOW_SIZE  = 800;
const string WINDOW_TITLE = "Erica Mauriz Barundia"; 

// ===== MAIN =====
int main()
{
    constexpr chrono::nanoseconds timestep(16ms); // ~60 FPS
    
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

    // Compile shaders
    GLuint shader = compileShaders("shaders/sample.vert", "shaders/sample.frag");

    // Build sphere
    Sphere sphere;
    sphere.build(1.0f, 72, 36);  // radius=1, smoother
    sphere.upload();

    // OpenGL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);  // dark background

    // ESC to close
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    using clock = chrono::high_resolution_clock;
    auto curr_time = clock::now();
    auto prev_time = curr_time;
    chrono::nanoseconds curr_ns(0);

    // ===== RENDER LOOP =====
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        // --- Orthographic projection ---
        // Covers -3 to 3 on each axis, square aspect ratio
        float orthoSize = 2.5f;
        glm::mat4 projection = glm::ortho(
            -orthoSize, orthoSize,   // left, right
            -orthoSize, orthoSize,   // bottom, top
            0.1f, 100.0f             // near, far
        );

        // --- Camera looking straight at origin ---
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),   // camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // look at origin (sphere center)
            glm::vec3(0.0f, 1.0f, 0.0f)    // up vector
        );

        // --- Sphere at center, no transform ---
        glm::mat4 transform = glm::mat4(1.0f);

        // Upload matrices
        glUniformMatrix4fv(
            glGetUniformLocation(shader, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(
            glGetUniformLocation(shader, "view"),
            1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(
            glGetUniformLocation(shader, "transform"),
            1, GL_FALSE, glm::value_ptr(transform));

        // Draw sphere
        sphere.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // --- Frame timing ---
        curr_time = clock::now();
        auto dur = chrono::duration_cast<chrono::nanoseconds>(curr_time - prev_time);
        prev_time = curr_time;
    }

    // Cleanup
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}