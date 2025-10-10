#include "raytracer.h"

Raytracer::Raytracer(const char* computeShaderFile){
    changeProgram(computeShaderFile);
    viewportSizeId = glGetUniformLocation(program, "viewportSize");
    cameraPosId = glGetUniformLocation(program, "cameraPos");
    lookDirId = glGetUniformLocation(program, "lookDir");
    upVectorId = glGetUniformLocation(program, "upVector");
    vFovId = glGetUniformLocation(program, "vFov");
    //Background
    backgroundTypeId = glGetUniformLocation(program, "backgroundType");
    backgroundGridSizeId = glGetUniformLocation(program, "backgroundGridSize");
    backgroundDistanceId = glGetUniformLocation(program, "backgroundDistance");
    backgroundColorAId = glGetUniformLocation(program, "backgroundColorA");
    backgroundColorBId = glGetUniformLocation(program, "backgroundColorB");

    glGenTextures(1, &textureOutput);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &debugOutput);
    glBindTexture(GL_TEXTURE_2D, debugOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupMax[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupMax[1]);
}

void Raytracer::update(int textureWidth, int textureHeight){
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, textureOutput); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, debugOutput); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, debugOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform2f(viewportSizeId, textureWidth, textureHeight);
}

void Raytracer::dispatch(int x, int y){
    //std::cout << "DISPATCHED" << std::endl;
    glUseProgram(program);
    //Camera
    glUniform3f(cameraPosId, camera.position.x, camera.position.y, camera.position.z);
    glUniform3f(lookDirId, camera.look.x, camera.look.y, camera.look.z);
    glUniform3f(upVectorId, camera.up.x, camera.up.y, camera.up.z);
    glUniform1f(vFovId, camera.verticalFOV);
    //Background
    glUniform1i(backgroundTypeId, background.type); 
    glUniform2f(backgroundGridSizeId, background.gridSize.x, background.gridSize.y);
    glUniform1f(backgroundDistanceId, background.distance);
    glUniform3f(backgroundColorAId, background.colorA.x, background.colorA.y, background.colorA.z);  
    glUniform3f(backgroundColorBId, background.colorB.x, background.colorB.y, background.colorB.z);  
    glDispatchCompute(std::min(x, workGroupMax[0]), std::min(y, workGroupMax[1]), 1);
}

void Raytracer::changeProgram(const char* filename){
    if(program != 0){
        glDeleteProgram(program);
    }
    program = linkProgram({
        compileShader(filename, GL_COMPUTE_SHADER)
    });
}

GLuint Raytracer::getOutputTexture(){
    return textureOutput;
}
GLuint Raytracer::getDebugTexture(){
    return debugOutput;
}
