#pragma once
#include "glm/vec3.hpp"

struct Camera {
    glm::vec3 position;
    glm::vec3 look;
    glm::vec3 up;
    float verticalFOV = 10;
};
