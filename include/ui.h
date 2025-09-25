#pragma once
#include <string>
#include <glad/glad.h>
//Imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//Project
#include "settings.h"
#include "planet.h"

class UI {
public:
    void begin();
    void settings();
    void viewport(GLuint framebufferTexture);
    void universe(std::vector<Planet>* ref);
    void end();
    Settings* getSettings();
    //Flags: set by the ui, cleared by the outside
    bool shouldDispatchFlag();
    void clearDispatchFlag();
    bool shouldUpdateUniverseFlag();
    void clearUpdateUniverseFlag();
private:
    Settings active;
    int dirtyResolution[2] = {DEFAULT_WIDTH, DEFAULT_HEIGHT};
    bool shouldDispatch = true;
    bool shouldUpdateUniverse = false;
};
