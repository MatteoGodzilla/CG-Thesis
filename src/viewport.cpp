#include "viewport.h"

Viewport::Viewport(){
    //Vertex
    display = linkProgram({
        compileShader("vertex.shader", GL_VERTEX_SHADER),
        compileShader("fragment.shader", GL_FRAGMENT_SHADER)
    });

    raytracerSizeId = glGetUniformLocation(display,"raytracerOutputSize");
    viewportSizeId = glGetUniformLocation(display,"viewportSize");

    //VAO    
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);
    //Position
    GLuint quadPositionVBO;
    glGenBuffers(1, &quadPositionVBO);
    float positions[] = {
        -1, 1,
        1, 1, 
        1, -1,
        -1, -1
    };
    glBindBuffer(GL_ARRAY_BUFFER, quadPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    //Indicies
    GLuint quadIndiciesVBO;
    glGenBuffers(1, &quadIndiciesVBO);
    unsigned int indicies[] = {
        0, 1, 2,
        2, 3, 0
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndiciesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
    //uvs
    GLuint quadTextureVBO;
    glGenBuffers(1, &quadTextureVBO);
    float uvs[] = {
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0,
        0.0, 0.0
    };
    glBindBuffer(GL_ARRAY_BUFFER, quadTextureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
}

void Viewport::update(Settings* settings, GLuint textureId){
    glUseProgram(display);
    glUniform2f(raytracerSizeId, settings->resolution[0], settings->resolution[1]);
    glUniform2f(viewportSizeId, settings->viewportSize[0], settings->viewportSize[1]);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Viewport::draw(){
    glUseProgram(display);
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
