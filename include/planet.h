#pragma once
#include <string>
#include <sstream>
#include "glm/vec3.hpp"

struct Planet{
    std::string name;
    //Basic info
    glm::vec3 position; //x,y,z, in meters
    glm::vec3 northVector; //normalized
    glm::vec3 zeroDegree; //normalized
    float radius; //in meters
    float mass; //in kilograms
    //Graphic stuff
    glm::vec3 ambient; //r,g,b
    glm::vec3 diffuse; //r,g,b
    glm::vec3 emission; //r,g,b
    float luminosity; //watts
    std::string albedoTextureFile;
};

struct PlanetGLSL {
    alignas(16) glm::vec3 position; //x,y,z, in meters
    alignas(16) glm::vec3 northVector; //normalized
    alignas(16) glm::vec3 zeroDegree; //normalized
    float radius; //in meters
    float mass; //in kilograms
    alignas(16) glm::vec3 ambient; //r,g,b
    alignas(16) glm::vec3 diffuse; //r,g,b
    alignas(16) glm::vec3 emission; //r,g,b
    float luminosity; //watts
};

std::vector<PlanetGLSL> planetsToGLSL(std::vector<Planet>* ref);

Planet deserialize(std::string s);
std::string serialize(Planet p);
