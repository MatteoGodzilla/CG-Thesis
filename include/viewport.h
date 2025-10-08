#pragma once
#include <map>
#include <glad/glad.h>
#include "settings.h"
#include "shader.h"
#include "viewportFilter.h"

class Viewport {
public:
    Viewport();
    void update(Settings* settings, GLuint textureId);
    void draw(ViewportFilter filter);
private:
    std::map<ViewportFilter, GLuint> programs;
    GLuint quadVAO;
    //Uniform ids
    GLuint raytracerSizeId;
    GLuint viewportSizeId;
};
