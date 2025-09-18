#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include "shader.h"

void GLAPIENTRY
MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if(type == GL_DEBUG_TYPE_ERROR){
        std::cerr << "GL ERROR: " << message << std::endl; 
    }
}

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(){
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tesi OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 2;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        glfwTerminate();
        return 3;
    }
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
    
    //GLuint rayTracer = linkProgram(computeShaders);
    GLuint rayTracer = linkProgram({
        compileShader("compute.shader", GL_COMPUTE_SHADER)
    });

    
    //Texture buffer 
    //TODO: make this dynamic based on viewport dimentions
    int textureWidth = WINDOW_WIDTH;
    int textureHeight = WINDOW_HEIGHT;
    GLuint textureOutput;
    glGenTextures(1, &textureOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    //Get maximum work group count
    //TODO: calculate best amount of work groups
    int workGroupMax[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupMax[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupMax[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupMax[2]);

    //std::cout << workGroupMax[0] << ", " << workGroupMax[0] << ", " << workGroupMax[0] << ", " << std::endl;

    //Vertex
    GLuint display = linkProgram({
        compileShader("vertex.shader", GL_VERTEX_SHADER),
        compileShader("fragment.shader", GL_FRAGMENT_SHADER)
    });

    //VAO    
    GLuint quadVAO;
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


    //Buffer
    GLuint transmissionBuffer;
    glGenBuffers(1, &transmissionBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);
    srand(time(NULL)); 
    std::vector<float> transmissionData;

    for(int i = 0; i < 5 * 3; i++){
        float v = (float)rand() / RAND_MAX;
        transmissionData.push_back(v);
        std::cout << i << ":" << v << std::endl; 
    }

    /*
    float transmissionData[] = {
        //x normalized, y normalized, r, g, b
        0.33, 0.33, 1.0, 1.0, 0.0,
        0.66, 0.66, 0.0, 1.0, 0.0
        //0.5, 0.5, 0, 0, 1,
    };
    */
    glBufferData(GL_SHADER_STORAGE_BUFFER, transmissionData.size() * sizeof(float), transmissionData.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, transmissionBuffer);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Dispatch compute shader
        glUseProgram(rayTracer);
        //TODO: calculate best amount of work groups
        glDispatchCompute((GLuint)textureWidth, (GLuint)textureHeight, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* Render here */
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(display);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureOutput);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

