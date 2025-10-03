#pragma once
#include <glad/glad.h>
#include "settings.h"
#include "shader.h"

class Viewport {
public:
    Viewport();
    void update(Settings* settings, GLuint textureId);
    void draw();
private:
    GLuint display;
    GLuint quadVAO;
    //Uniform ids
    GLuint raytracerSizeId;
    GLuint viewportSizeId;
};
