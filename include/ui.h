#pragma once
#include <string>
#include <iostream>
#include <glad/glad.h>
#include <cmath>
#include "glm/vec2.hpp"
//Imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//Project
#include "planet.h"
#include "camera.h"
#include "flag.h"
#include "background.h"
#include "viewportFilter.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define BUF_SIZE 512

class UI {
public:
    void begin();
    void menuBar();
    void quickActions();
    void copyDebugTexture(GLuint debugTexture);
    void viewport(GLuint framebufferTexture, GLuint computeTexture, std::vector<Planet>* ref);
    void universe(Camera *camera, Background* background, std::vector<Planet>* ref);
    void end();
    //Flags: set by the ui, cleared by the outside
    Flag dispatch = Flag(true);     //start render
    Flag loadUniverse;              //Trigger file loading dialog
    Flag saveUniverse;              //Save universe to file, directly
    Flag saveUniverseAs;            //Trigger file saving dialog for universe file
    Flag dirtyUniverse;             //Universe has been modified, but not saved to file
    Flag exportImage;               //Trigger file saving dialog for rendered image
    Flag loadCompute;               //Trigger file loading dialog for compute shader
    Flag outdatedRender;            //Viewport does not reflect planets
    bool alwaysDispatch = false;    //NOT A FLAG because of imgui
    glm::ivec2 resolution = {DEFAULT_WIDTH,DEFAULT_HEIGHT};
    glm::ivec2 viewportSize;
    //Flags
    ViewportFilter filter = VF_FILL;
private:
    ImVec2 mouseToComputePixel();
    //has to be an array because it gets edited directly by imgui
    int dirtyResolution[2] = {DEFAULT_WIDTH, DEFAULT_HEIGHT};
    char newName[BUF_SIZE] = {0};
    std::vector<float> debugBuffer;
    Flag validDebugBuffer;
};

