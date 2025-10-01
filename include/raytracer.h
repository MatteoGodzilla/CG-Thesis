#pragma once
#include <iostream>
#include "glm/vec3.hpp"
#include "shader.h"
#include "camera.h"

class Raytracer {
public:
    Raytracer();
    void update(int textureWidth, int textureHeight);
    void dispatch(int x, int y);
    GLuint getOutputTexture();
    //actual camera values
    struct Camera camera = {
        .position = {5,0,-5},
        .look = {-1,0,0},
        .up = {0,1,0},
        .verticalFOV = 10
    };
private: 
    GLuint textureOutput;
    GLuint program;
    GLuint viewportSizeId;
    GLuint cameraPosId;
    GLuint lookDirId;
    GLuint upVectorId;
    GLuint vFovId;
};
