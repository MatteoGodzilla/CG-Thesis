#include "raytracer.h"

Raytracer::Raytracer(){
    program = linkProgram({
        compileShader("compute.shader", GL_COMPUTE_SHADER)
    });
    viewportSizeId = glGetUniformLocation(program, "viewportSize");
    cameraPosId = glGetUniformLocation(program, "cameraPos");
    lookDirId = glGetUniformLocation(program, "lookDir");
    upVectorId = glGetUniformLocation(program, "upVector");
    vFovId = glGetUniformLocation(program, "vFov");
    gridSizeId = glGetUniformLocation(program, "gridSize");
    backgroundDistanceId = glGetUniformLocation(program, "backgroundDistance");
    glGenTextures(1, &textureOutput);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void Raytracer::update(int textureWidth, int textureHeight){
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, textureOutput); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform2f(viewportSizeId, textureWidth, textureHeight);
}

void Raytracer::dispatch(int x, int y){
    //std::cout << "DISPATCHED" << std::endl;
    glUseProgram(program);
    glUniform3f(cameraPosId, camera.position.x, camera.position.y, camera.position.z);
    glUniform3f(lookDirId, camera.look.x, camera.look.y, camera.look.z);
    glUniform3f(upVectorId, camera.up.x, camera.up.y, camera.up.z);
    glUniform1f(vFovId, camera.verticalFOV);
    glUniform2f(gridSizeId, background.gridSize.x, background.gridSize.y);
    glUniform1f(backgroundDistanceId, background.distance);
    glDispatchCompute((GLuint)x, (GLuint)y, 1);
}

GLuint Raytracer::getOutputTexture(){
    return textureOutput;
}
