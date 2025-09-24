//System
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <vector>
//Project
#include "shader.h"
#include "ui.h"
#include "settings.h"
#include "raytracer.h"
#include "framebuffer.h"
#include "planet.h"

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

    UI ui;
    Raytracer raytracer;
    Settings* defaultSettings = ui.getSettings();
    raytracer.update(defaultSettings->resolution[0], defaultSettings->resolution[1]);

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

    /*
    srand(time(NULL)); 
    std::vector<float> transmissionData;

    //x normalized, y normalized, r, g, b
    for(int i = 0; i < 5 * 3; i++){
        float v = (float)rand() / RAND_MAX;
        transmissionData.push_back(v);
        std::cout << i << ":" << v << std::endl; 
    }
    */

    std::vector<struct Planet> planets;
    planets.push_back({ 
        .position = {0,0,1},
        .color = {0,1,0},
        .radius = 5, 
        .mass = 0.75
    });


    glBufferData(GL_SHADER_STORAGE_BUFFER, planets.size() * sizeof(struct Planet), planets.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, transmissionBuffer);

    //Framebuffer
    Framebuffer framebuffer;
    framebuffer.update(WINDOW_WIDTH, WINDOW_HEIGHT);

    //Uniform stuff for the manual render
    GLint raytracerSizeId = glGetUniformLocation(display,"raytracerOutputSize");
    GLint viewportSizeId = glGetUniformLocation(display,"viewportSize");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        //---Renderer---
        //TODO: calculate best amount of work groups
        Settings* settings = ui.getSettings();
        if(ui.shouldDispatchFlag()){
            int w = settings->resolution[0];
            int h = settings->resolution[1];
            raytracer.update(w, h);
            raytracer.dispatch(w, h);
            ui.clearDispatchFlag();
        }

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //---From compute shader output to framebuffer---
        framebuffer.bind();
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(display);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glUniform2f(raytracerSizeId, settings->resolution[0], settings->resolution[1]);
        glUniform2f(viewportSizeId, settings->viewportSize[0], settings->viewportSize[1]);
        glBindTexture(GL_TEXTURE_2D, raytracer.getOutputTexture());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        framebuffer.unbind();
      
        //---UI---
        ui.begin();
        //ImGui::ShowDemoWindow(); 
        ui.settings();
        ui.viewport(framebuffer.getColorTexture());
        ui.end();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

