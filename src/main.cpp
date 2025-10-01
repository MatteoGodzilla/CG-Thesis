//System
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <iomanip> 
//External
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
//Project
#include "shader.h"
#include "ui.h"
#include "settings.h"
#include "raytracer.h"
#include "framebuffer.h"
#include "planet.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define UNIVERSE "universe.txt"

void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
    if(type == GL_DEBUG_TYPE_ERROR){
        std::cerr << "GL ERROR: " << message << std::endl; 
    } else {
        std::cout << "LOG: " << message << std::endl;
    }
}

void deserializeAll(const char* filename, Camera* camera, std::vector<Planet>* ref){
    std::ifstream inFile(filename);
    if(inFile.is_open()){
        std::string temp;
        getline(inFile,temp); //Camera 
        inFile >> camera->position.x >> camera->position.y >> camera->position.z;
        inFile >> camera->look.x >> camera->look.y >> camera->look.z;
        inFile >> camera->up.x >> camera->up.y >> camera->up.z;
        getline(inFile, temp); //consume the newline after up vector
        getline(inFile, temp); //consume the empty line afterwards 
        getline(inFile, temp); //get the number of planets
        int count = std::stoi(temp);
        for(int i = 0; i < count; i++){
            Planet res = {"##", {0,0,0}, {0,0,0}, 0, 0};
            getline(inFile, res.name);
            inFile >> res.position.x >> res.position.y >> res.position.z;
            inFile >> res.color.x >> res.color.y >> res.color.z;
            inFile >> res.radius;
            inFile >> res.mass;
            ref->push_back(res);
            getline(inFile, temp); //consume the newline after the radius
            getline(inFile, temp); //consume the empty line afterwards
        }
        inFile.close();
    }
}

void serializeAll(const char* filename, Camera* camera, std::vector<Planet>* ref){
    std::ofstream outFile(filename);
    if(outFile.is_open()){
        outFile << "Camera" << std::endl;
        outFile << camera->position.x << " " << camera->position.y << " " << camera->position.z << std::endl;
        outFile << camera->look.x << " " << camera->look.y << " " << camera->look.z << std::endl;
        outFile << camera->up.x << " " << camera->up.y << " " << camera->up.z << std::endl;
        outFile << std::endl;
        outFile << ref->size() << std::endl;
        for(auto& p : *ref){
            outFile << p.name << std::endl;
            outFile << p.position.x << " " << p.position.y << " " <<  p.position.z << std::endl;
            outFile << p.color.x << " " <<  p.color.y << " " <<  p.color.z << std::endl;
            outFile << p.radius << std::endl;
            outFile << p.mass << std::endl;
            outFile << std::endl;
        }
        outFile.close();
    }
}

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

    std::vector<struct Planet> planets;
    deserializeAll(UNIVERSE, &(raytracer.camera), &planets);

    std::vector<PlanetGLSL> converted = planetsToGLSL(&planets);
    glBufferData(GL_SHADER_STORAGE_BUFFER, converted.size() * sizeof(struct PlanetGLSL), converted.data(), GL_STATIC_READ);
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
        Settings* settings = ui.getSettings();
        if(ui.dispatch.getState()){
            int w = settings->resolution[0];
            int h = settings->resolution[1];
            raytracer.update(w, h);
            raytracer.dispatch(w, h);
            ui.dispatch.clear();
        }

        if(ui.updateUniverse.getState()){
            std::vector<PlanetGLSL> converted = planetsToGLSL(&planets);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, transmissionBuffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, converted.size() * sizeof(struct PlanetGLSL), converted.data(), GL_STATIC_READ);
            ui.updateUniverse.clear();
        }

        if(ui.saveUniverse.getState()){
            serializeAll(UNIVERSE, &(raytracer.camera), &planets);
        }
    
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        if(ui.exportImage.getState()){
            //Get string of file to save to (autogenerated)
            std::ostringstream filename;
            auto t = std::time(nullptr);
            filename << std::put_time(std::localtime(&t), "output_%d-%m-%Y_%H-%M-%S.png");
            //Get texture data
            int w = settings->resolution[0];
            int h = settings->resolution[1];
            std::vector<unsigned char> output (w * h * 3); //rgb
            glBindTexture(GL_TEXTURE_2D, raytracer.getOutputTexture());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &output[0]);
            stbi_write_png(filename.str().c_str(), w, h, 3, output.data(), w * 3);
            std::cout << "Saved to " << filename.str() << std::endl;
            ui.exportImage.clear();
        }

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
        ui.universe(&(raytracer.camera), &planets);
        ui.viewport(framebuffer.getColorTexture());
        ui.end();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

