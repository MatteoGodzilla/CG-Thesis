#pragma once
#include <map>
#include <glad/glad.h>
#include "shader.h"
#include "viewportFilter.h"

class Viewport {
public:
    Viewport();
    //Render width, render height, viewport width, viewport height
    void update(GLuint textureId, int rw, int rh, int vw, int vh, ViewportFilter filter);
    void draw(ViewportFilter filter);
private:
    std::map<ViewportFilter, GLuint> programs;
    GLuint quadVAO;
    //Uniform ids
    GLuint raytracerSizeId;
    GLuint viewportSizeId;
};
