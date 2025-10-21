#include "raytracer.h"

Raytracer::Raytracer(const char* computeShaderFile){
    changeProgram(computeShaderFile);
    //uniform locations
    viewportSizeId = glGetUniformLocation(program, "viewportSize");
    cameraPosId = glGetUniformLocation(program, "cameraPos");
    lookDirId = glGetUniformLocation(program, "lookDir");
    upVectorId = glGetUniformLocation(program, "upVector");
    vFovId = glGetUniformLocation(program, "vFov");
    //Background uniform locations
    backgroundTypeId = glGetUniformLocation(program, "backgroundType");
    backgroundGridSizeId = glGetUniformLocation(program, "backgroundGridSize");
    backgroundDistanceId = glGetUniformLocation(program, "backgroundDistance");
    backgroundColorAId = glGetUniformLocation(program, "backgroundColorA");
    backgroundColorBId = glGetUniformLocation(program, "backgroundColorB");
    //Output 1: main texture
    glGenTextures(1, &textureOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //Output 2: debug texture
    glGenTextures(1, &debugOutput);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, debugOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //Transmission buffer for planets 
    glGenBuffers(1, &transmissionBuffer);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer); //?
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, transmissionBuffer);

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &(workGroupMax.x));
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &(workGroupMax.y));
}

void Raytracer::update(int textureWidth, int textureHeight, std::vector<Planet>* planets){
    glUseProgram(program);
    //Set up output textures
    glBindTexture(GL_TEXTURE_2D, textureOutput); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, debugOutput); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, debugOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glUniform2f(viewportSizeId, textureWidth, textureHeight);
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
    //Planets
    std::vector<PlanetGLSL> converted = planetsToGLSL(planets);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, converted.size() * sizeof(PlanetGLSL), converted.data(), GL_STATIC_READ);
    //Planets' textures
    glDeleteTextures(1, &planetTextures);
    glDeleteTextures(1, &planetTextureSize);

    glGenTextures(1, &planetTextures);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D_ARRAY, planetTextures);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glGenTextures(1, &planetTextureSize);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, planetTextureSize);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    int maxWidth = 0;
    int maxHeight = 0;
    for(size_t i = 0; i < planets->size(); i++){
        std::string albedoFile = planets->at(i).albedoTextureFile;
        if(!albedoFile.empty()){
            int width;
            int height;
            int channels;
            stbi_info(albedoFile.c_str(), &width, &height, &channels);
            maxWidth = std::max(maxWidth, width);
            maxHeight = std::max(maxHeight, height);
        }
    }
    glBindTexture(GL_TEXTURE_2D_ARRAY, planetTextures);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, maxWidth, maxHeight, planets->size());
    glBindTexture(GL_TEXTURE_2D, planetTextureSize);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, 2, planets->size() );
    for(size_t i = 0; i < planets->size(); i++){
        std::string albedoFile = planets->at(i).albedoTextureFile;
        if(!albedoFile.empty()){
            std::cout << "Trying to load " << albedoFile << std::endl;
            int x;
            int y;
            int channels;
            unsigned char* data = stbi_load(albedoFile.c_str(), &x, &y, &channels, 3);
            if(data != nullptr){
                std::cout << "Loaded planet " << i << std::endl;
                glBindTexture(GL_TEXTURE_2D_ARRAY, planetTextures);
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, x, y, 1, GL_RGB, GL_UNSIGNED_BYTE, data); 
                glBindTexture(GL_TEXTURE_2D, planetTextureSize);
                float texRelativeSize[2] = {float(x) / maxWidth, float(y) / maxHeight};
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, 2, 1, GL_RG, GL_FLOAT, texRelativeSize);
                stbi_image_free(data);
            }
        }
    }

    //Dispatch size
    dispatchSize.x = std::min(textureWidth, workGroupMax.x);
    dispatchSize.y = std::min(textureHeight, workGroupMax.y);
}

void Raytracer::dispatch(){
    glUseProgram(program);
    glDispatchCompute(dispatchSize.x, dispatchSize.y, 1);
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
