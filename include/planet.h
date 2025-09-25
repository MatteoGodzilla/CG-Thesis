#pragma once
#include "glm/vec3.hpp"

struct Planet {
    alignas(16) glm::vec3 position; //x,y,z, in meters
    alignas(16) glm::vec3 color; //r,g,b
    float radius; //in meters
    float mass; //in kilograms
};
