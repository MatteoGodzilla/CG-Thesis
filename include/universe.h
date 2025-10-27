#pragma once
#include <vector>
#include "camera.h"
#include "background.h"
#include "planet.h"

struct Universe {
    Camera camera = {
        .position = {5,0,-5},
        .look = {-1,0,0},
        .up = {0,1,0},
        .verticalFOV = 10
    };
    Background background {
        .type = BG_SOLID,
        .gridSize = glm::vec2(1e6, 1e6),
        .distance = 1e6,
        .colorA = glm::vec3(0, 0.1, 0.2),
        .colorB = glm::vec3(0,0,0)
    };
    std::vector<Planet> planets;
};
