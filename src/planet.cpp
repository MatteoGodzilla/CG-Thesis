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


Planet deserialize(std::string s){
    std::istringstream stream(s);
    Planet res = {"", {0,0,0}, {0,0,0}, 0, 0};
    getline(stream, res.name);
    stream >> res.position.x;
    stream >> res.position.y;
    stream >> res.position.z;
    stream >> res.color.x;
    stream >> res.color.y;
    stream >> res.color.z;
    stream >> res.radius;
    stream >> res.mass;
    return res;
}

std::string serialize(Planet p){
    std::stringstream res;
    res << p.name << std::endl;
    res << p.position.x << std::endl;
    res << p.position.y << std::endl;
    res << p.position.z << std::endl;
    res << p.color.x << std::endl;
    res << p.color.y << std::endl;
    res << p.color.z << std::endl;
    res << p.radius << std::endl;
    res << p.mass << std::endl;
    return res.str();
}
