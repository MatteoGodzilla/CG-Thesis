#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

enum BackgroundType : int {
    BG_SOLID,
    BG_GRID,
    BG_SIZE
};

struct Background {
    BackgroundType type;
    glm::vec2 gridSize;
    float distance;
    glm::vec3 colorA;
    glm::vec3 colorB;
};
