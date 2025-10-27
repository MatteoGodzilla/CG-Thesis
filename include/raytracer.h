#pragma once
#include <iostream>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "stb_image.h"
#include "shader.h"
#include "universe.h"

#define COMPUTE_SHADER "shaders/compute.shader" 

class Raytracer {
public:
    Raytracer(const char* computeShaderFile);
    void update(int textureWidth, int textureHeight, Universe* universe);
    void changeProgram(const char* filename);
    void dispatch();
    GLuint getOutputTexture();
    GLuint getDebugTexture();
    //actual camera values
    //TODO: maybe move these two outside? so that Raytracer only deals with sending to the GPU?
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
