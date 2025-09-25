#pragma once
#include <iostream>
#include "glm/vec3.hpp"
#include "shader.h"

class Raytracer {
public:
    Raytracer();
    void update(int textureWidth, int textureHeight);
    void dispatch(int x, int y);
    GLuint getOutputTexture();
private: 
    GLuint textureOutput;
    GLuint program;
    GLuint viewportSizeId;
    GLuint cameraPosId;
    GLuint lookDirId;
    GLuint upVectorId;
    GLuint vFovId;
    //actual camera values
    glm::vec3 position = {0,0,0};
    glm::vec3 look = {0,0,-1};
    glm::vec3 up = {0,1,0};
    float verticalFov = 10;
};
