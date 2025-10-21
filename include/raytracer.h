#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "stb_image.h"
#include "shader.h"
#include "camera.h"
#include "background.h"
#include "planet.h"

class Raytracer {
public:
    Raytracer(const char* computeShaderFile);
    void update(int textureWidth, int textureHeight, std::vector<Planet>* planets);
    void changeProgram(const char* filename);
    void dispatch();
    GLuint getOutputTexture();
    GLuint getDebugTexture();
    //actual camera values
    Camera camera = {
        .position = {5,0,-5},
        .look = {-1,0,0},
        .up = {0,1,0},
        .verticalFOV = 10
    };
    Background background {
        .type = BG_SOLID,
        .gridSize = glm::vec2(1e6, 1e6),
        .distance = 1e6,
        .colorA = glm::vec3(0, 0.1, 0.2),
        .colorB = glm::vec3(0,0,0)
    };
private: 
    GLuint textureOutput;
    GLuint debugOutput;
    GLuint transmissionBuffer;
    GLuint planetTextures;
    GLuint planetTextureSize;
    GLuint program = 0;
    GLuint viewportSizeId;
    //Camera stuff
    GLuint cameraPosId;
    GLuint lookDirId;
    GLuint upVectorId;
    GLuint vFovId;
    //Background stuff
    GLuint backgroundTypeId;
    GLuint backgroundGridSizeId;
    GLuint backgroundDistanceId;
    GLuint backgroundColorAId;
    GLuint backgroundColorBId;
    //other
    glm::ivec2 workGroupMax;
    glm::ivec2 dispatchSize;
};
