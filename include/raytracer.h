#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "shader.h"
#include "camera.h"
#include "background.h"

class Raytracer {
public:
    Raytracer();
    void update(int textureWidth, int textureHeight);
    void dispatch(int x, int y);
    GLuint getOutputTexture();
    //actual camera values
    Camera camera = {
        .position = {5,0,-5},
        .look = {-1,0,0},
        .up = {0,1,0},
        .verticalFOV = 10
    };
    //TODO: maybe make a struct instead?
    Background background {
        .gridSize = glm::vec2(1e6, 1e6),
        .distance = 1e6
    };
private: 
    GLuint textureOutput;
    GLuint program;
    GLuint viewportSizeId;
    GLuint cameraPosId;
    GLuint lookDirId;
    GLuint upVectorId;
    GLuint vFovId;
    GLuint gridSizeId;
    GLuint backgroundDistanceId;
    int workGroupMax[2] = {0,0};
};
