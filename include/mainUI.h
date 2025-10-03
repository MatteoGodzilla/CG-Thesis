#pragma once
//System
//#include <iostream>
//#include <time.h>
//#include <stdlib.h>
//#include <vector>
#include <iomanip> 
//External
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image_write.h>
//Project
#include "shader.h"
#include "ui.h"
#include "settings.h"
#include "raytracer.h"
#include "framebuffer.h"
#include "planet.h"
#include "viewport.h"
#include "serialization.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define UNIVERSE "universe.txt"

int mainUI();
