#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include <fstream>

GLuint compileShader(const char* sourceFile, GLenum type);
GLuint linkProgram(std::vector<GLuint> shaders);
