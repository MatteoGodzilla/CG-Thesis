#include "planet.h"

std::vector<PlanetGLSL> planetsToGLSL(std::vector<Planet>* ref){
    std::vector<PlanetGLSL> result;
    for(size_t i = 0; i < ref->size(); i++){
        Planet& p = ref->at(i);
        result.push_back({
            .position = p.position,
            .color = p.color,
            .radius = p.radius,
            .mass = p.mass
        });
    }
    return result;
}
