#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "planet.h"
#include "camera.h"
#include "background.h"

void deserializeAll(const char* filename, Camera* camera, Background* background, std::vector<Planet>* ref);
void serializeAll(const char* filename, Camera* camera, Background* background, std::vector<Planet>* ref);
