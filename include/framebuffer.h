#pragma once
#include <stdlib.h>
#include <glad/glad.h>

class Framebuffer {
public:
    Framebuffer();
    void update(int width, int height);
    void bind();
    void unbind();
    GLuint getColorTexture();
private:
    GLuint id;
    GLuint colorTexture;
};
