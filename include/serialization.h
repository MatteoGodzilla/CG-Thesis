#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "planet.h"
#include "camera.h"
#include "background.h"

void deserializeAll(std::istream& input, Camera* camera, Background* background, std::vector<Planet>* ref);
void serializeAll(std::ostream& output, Camera* camera, Background* background, std::vector<Planet>* ref);
