#include "framebuffer.h"

Framebuffer::Framebuffer(){
    glGenFramebuffers(1, &id);
    glGenTextures(1, &colorTexture);
    bind();
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    unbind();
}

void Framebuffer::update(int width, int height){
    bind();
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    unbind();
}

void Framebuffer::bind(){
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void Framebuffer::unbind(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Framebuffer::getColorTexture(){
    return colorTexture;
}
