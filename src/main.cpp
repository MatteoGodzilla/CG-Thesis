//System
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <vector>
//Imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//Project
#include "shader.h"


void GLAPIENTRY
MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if(type == GL_DEBUG_TYPE_ERROR){
        std::cerr << "GL ERROR: " << message << std::endl; 
    } else {
        std::cout << "LOG: " << message << std::endl;
    }
}

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

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

    //Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    //GLuint rayTracer = linkProgram(computeShaders);
    GLuint rayTracer = linkProgram({
        compileShader("compute.shader", GL_COMPUTE_SHADER)
    });

    //Texture buffer 
    //TODO: make this dynamic based on viewport dimentions
    int textureWidth = 800;
    int textureHeight = 600;
    GLuint textureOutput;
    glGenTextures(1, &textureOutput);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, textureOutput, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    //Get maximum work group count
    //TODO: calculate best amount of work groups
    int workGroupMax[3];
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupMax[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupMax[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupMax[2]);

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
        -0.5, 0.5,
        0.5, 0.5, 
        0.5, -0.5,
        -0.5, -0.5
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

    //x normalized, y normalized, r, g, b
    for(int i = 0; i < 5 * 3; i++){
        float v = (float)rand() / RAND_MAX;
        transmissionData.push_back(v);
        std::cout << i << ":" << v << std::endl; 
    }

    glBufferData(GL_SHADER_STORAGE_BUFFER, transmissionData.size() * sizeof(float), transmissionData.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, transmissionBuffer);

    //Framebuffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //Framebuffer color texture
    GLuint fboColor;
    glGenTextures(1, &fboColor);
    glBindTexture(GL_TEXTURE_2D, fboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Poll for and process events */
        glfwPollEvents();

        //---ImGui---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //---Renderer---
        // Dispatch compute shader
        glUseProgram(rayTracer);
        //TODO: calculate best amount of work groups
        glDispatchCompute((GLuint)textureWidth, (GLuint)textureHeight, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* Render here */
        glClearColor(1,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(display);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureOutput);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        ImGui::ShowDemoWindow(); // Show demo window! :)
                                
        ImGui::Begin("ViewPort");
        {
            ImGui::Text("AAAAAAAAAAAAAAA");
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, fboColor);
            ImGui::Image((void*)fboColor, ImVec2(400, 300));
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}

