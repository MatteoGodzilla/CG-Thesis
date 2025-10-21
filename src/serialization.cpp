#include "serialization.h"

void deserializeAll(std::istream& inFile, Camera* camera, Background* background, std::vector<Planet>* ref){
    nlohmann::json res;
    inFile >> res;

    std::string cameraKey = "camera";
    nlohmann::json cameraPos = res[cameraKey]["position"];
    camera->position = {cameraPos[0], cameraPos[1], cameraPos[2]}; 
    nlohmann::json cameraLook = res[cameraKey]["look"];
    camera->look = {cameraLook[0], cameraLook[1], cameraLook[2]}; 
    nlohmann::json cameraUp = res[cameraKey]["up"];
    camera->up = {cameraUp[0], cameraUp[1], cameraUp[2]}; 
    camera->verticalFOV = res[cameraKey]["fov"];

    std::string backgroundKey = "background";
    background->type = res[backgroundKey]["type"];
    nlohmann::json backgroundGrid = res[backgroundKey]["gridSize"];
    background->gridSize = {backgroundGrid[0], backgroundGrid[1]};
    background->distance = res[backgroundKey]["distance"];
    nlohmann::json backgroundColorA = res[backgroundKey]["colorA"];
    background->colorA = {backgroundColorA[0], backgroundColorA[1], backgroundColorA[2]};
    nlohmann::json backgroundColorB = res[backgroundKey]["colorB"];
    background->colorB = {backgroundColorB[0], backgroundColorB[1], backgroundColorB[2]};

    for(auto& planet : res["planets"]){
        Planet res = {"##", {0,0,0}, {0,0,0}, {0,0,0}, 0, 0, {0,0,0}, {0,0,0}, {0,0,0}, 0};
        res.name = planet["name"];
        nlohmann::json planetPos = planet["position"];
        res.position = {planetPos[0], planetPos[1], planetPos[2]};
        nlohmann::json planetNorth = planet["northVector"];
        res.northVector = {planetNorth[0], planetNorth[1], planetNorth[2]};
        nlohmann::json planetZeroDeg = planet["zeroDegree"];
        res.zeroDegree = {planetZeroDeg[0], planetZeroDeg[1], planetZeroDeg[2]};
        res.radius = planet["radius"];
        res.mass = planet["mass"];
        nlohmann::json planetAmbient = planet["ambient"];
        res.ambient = {planetAmbient[0], planetAmbient[1], planetAmbient[2]};
        nlohmann::json planetDiffuse = planet["diffuse"];
        res.diffuse = {planetDiffuse[0], planetDiffuse[1], planetDiffuse[2]};
        nlohmann::json planetEmission = planet["emission"];
        res.emission = {planetEmission[0], planetEmission[1], planetEmission[2]};
        res.luminosity = planet["luminosity"];
        res.albedoTextureFile = planet["albedoTexture"];
        ref->push_back(res);
    }
}

void serializeAll(std::ostream& outFile, Camera* camera, Background* background, std::vector<Planet>* ref){
    nlohmann::json j;
    j["camera"] = {
        {"position", {camera->position.x, camera->position.y, camera->position.z}},
        {"look", {camera->look.x, camera->look.y, camera->look.z}},
        {"up", {camera->up.x, camera->up.y, camera->up.z}},
        {"fov", camera->verticalFOV}
    };

    j["background"] = {
        {"type", background->type},
        {"gridSize", {background->gridSize.x, background->gridSize.y}},
        {"distance", background->distance},
        {"colorA", {background->colorA.r, background->colorA.g, background->colorA.b}},
        {"colorB", {background->colorB.r, background->colorB.g, background->colorB.b}}
    };

    for(auto& planet : *ref){
        j["planets"].push_back({
            {"name", planet.name},
            {"position", {planet.position.x, planet.position.y, planet.position.z }},
            {"northVector", {planet.northVector.x, planet.northVector.y, planet.northVector.z }},
            {"zeroDegree", {planet.zeroDegree.x, planet.zeroDegree.y, planet.zeroDegree.z }},
            {"radius", planet.radius},
            {"mass", planet.mass},
            {"ambient", {planet.ambient.x, planet.ambient.y, planet.ambient.z }},
            {"diffuse", {planet.diffuse.x, planet.diffuse.y, planet.diffuse.z }},
            {"emission", {planet.emission.x, planet.emission.y, planet.emission.z }},
            {"luminosity", planet.luminosity},
            {"albedoTexture", planet.albedoTextureFile},
        });
    }
    outFile << std::setw(4) << j << std::endl;
}
