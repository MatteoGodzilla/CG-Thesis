#pragma once
#include "shader.h"
#include <iostream>

class Raytracer {
public:
    Raytracer();
    void update(int textureWidth, int textureHeight);
    void dispatch(int x, int y);
    GLuint getOutputTexture();
private: 
    GLuint textureOutput;
    GLuint program;
};
