#pragma once
#include <string>
#include <iostream>
#include <glad/glad.h>
#include "glm/vec2.hpp"
//Imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//Project
#include "settings.h"
#include "planet.h"
#include "camera.h"
#include "flag.h"
#include "background.h"


#define BUF_SIZE 512

class UI {
public:
    void begin();
    void settings();
    void viewport(GLuint framebufferTexture, GLuint computeTexture, std::vector<Planet>* ref);
    void universe(Camera *camera, Background* background, std::vector<Planet>* ref);
    void end();
    Settings* getSettings();
    //Flags: set by the ui, cleared by the outside
    Flag dispatch = Flag(true);
    Flag updateUniverse;
    Flag loadUniverse;
    Flag saveUniverse;
    Flag dirtyUniverse;
    Flag exportImage;
    Flag loadCompute;
private:
    ImVec2 mouseToComputePixel();
    Settings active;
    int dirtyResolution[2] = {DEFAULT_WIDTH, DEFAULT_HEIGHT};
    char newName[BUF_SIZE] = {0};
};

