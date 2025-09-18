#include "shader.h"

GLuint compileShader(const char* sourceFile, GLenum type){
    std::ifstream shaderFile = std::ifstream(sourceFile);
    if(!shaderFile.is_open()){
        return 0; //the same as createShader
    }
    std::string shaderSource = "";
    std::string temp;
    while(std::getline(shaderFile, temp)){
        shaderSource += temp + "\n";
    }
    GLuint result = glCreateShader(type);    
    const char* shaderSourceRaw = shaderSource.c_str();
    glShaderSource(result, 1, &shaderSourceRaw, NULL); 
    glCompileShader(result);
    return result;
}

GLuint linkProgram(std::vector<GLuint> shaders){
    GLuint result = glCreateProgram();
    for(auto& s : shaders){
        glAttachShader(result, s);
    }
    glLinkProgram(result);
    return result;
}

