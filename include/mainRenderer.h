#pragma once
#include <string>
#include <iostream>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "stb_image_write.h"
#include "serialization.h"
#include "raytracer.h"

int mainRenderer(int width, int height, std::istream& input, std::string output);
