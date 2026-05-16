/**
 * @file gdphysx.cpp
 * @author Erica Mauriz Barundia
 *
 * Assignment:
 * - Square window with name in title
 * - Orthographic camera
 * - Dark red sphere rendered at center
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headers/main.h"

using namespace std;

// ===== WINDOW =====
// Square window
const int   WINDOW_SIZE  = 800;
const string WINDOW_TITLE = "Erica Mauriz Barundia"; 

// ===== SHADER UTILS =====
string loadShaderFromFile(const string& path)
{
    fstream file(path);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot open shader: " << path << endl;
        return "";
    }
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint compileShaders(const string& vertPath, const string& fragPath)
{
    string vertStr = loadShaderFromFile(vertPath);
    string fragStr = loadShaderFromFile(fragPath);

    const char* vertSrc = vertStr.c_str();
    const char* fragSrc = fragStr.c_str();

    int  success;
    char log[512];

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, NULL);
    glCompileShader(vert);
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vert, 512, NULL, log);
        cerr << "Vertex shader error:\n" << log << endl;
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, NULL);
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(frag, 512, NULL, log);
        cerr << "Fragment shader error:\n" << log << endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, log);
        cerr << "Shader link error:\n" << log << endl;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    cout << "Shaders compiled OK" << endl;
    return program;
}

// ===== MAIN =====
int main()
{
    // Init GLFW
    if (!glfwInit()) {
        cerr << "GLFW init failed" << endl;
        return -1;
    }

    // Square window
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
    }

    // Cleanup
    sphere.cleanup();
    glDeleteProgram(shader);
    glfwTerminate();

    return 0;
}