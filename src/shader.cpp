#include "headers/shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

static string loadShaderFromFile(const string& path)
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