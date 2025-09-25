#pragma once
#include <string>
#include <sstream>
#include "glm/vec3.hpp"

struct Planet{
    std::string name;
    glm::vec3 position; //x,y,z, in meters
    glm::vec3 color; //r,g,b
    float radius; //in meters
    float mass; //in kilograms
};

struct PlanetGLSL {
    alignas(16) glm::vec3 position; //x,y,z, in meters
    alignas(16) glm::vec3 color; //r,g,b
    float radius; //in meters
    float mass; //in kilograms
};

std::vector<PlanetGLSL> planetsToGLSL(std::vector<Planet>* ref);

Planet deserialize(std::string s);
std::string serialize(Planet p);
