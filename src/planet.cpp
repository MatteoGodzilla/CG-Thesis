#include "planet.h"

std::vector<PlanetGLSL> planetsToGLSL(std::vector<Planet>* ref){
    std::vector<PlanetGLSL> result;
    for(size_t i = 0; i < ref->size(); i++){
        Planet& p = ref->at(i);
        result.push_back({
            .position = p.position,
            .northVector = p.northVector,
            .zeroDegree = p.zeroDegree,
            .radius = p.radius,
            .mass = p.mass,
            .ambient = p.ambient,
            .diffuse = p.diffuse,
            .emission = p.emission,
            .luminosity = p.luminosity,
        });
    }
    return result;
}
