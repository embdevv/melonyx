#pragma once
#include <glad/gl.h>
#include <string>

using namespace std;

GLuint compileShaders(const string& vertPath, const string& fragPath);
