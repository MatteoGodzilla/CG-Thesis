#include "serialization.h"

void deserializeAll(std::istream& inFile, Universe* ref){
    nlohmann::json res;
    inFile >> res;

    ref->planets.clear();

    std::string cameraKey = "camera";
    nlohmann::json cameraPos = res[cameraKey]["position"];
    ref->camera.position = {cameraPos[0], cameraPos[1], cameraPos[2]}; 
    nlohmann::json cameraLook = res[cameraKey]["look"];
    ref->camera.look = {cameraLook[0], cameraLook[1], cameraLook[2]}; 
    nlohmann::json cameraUp = res[cameraKey]["up"];
    ref->camera.up = {cameraUp[0], cameraUp[1], cameraUp[2]}; 
    ref->camera.verticalFOV = res[cameraKey]["fov"];

    std::string backgroundKey = "background";
    ref->background.type = res[backgroundKey]["type"];
    nlohmann::json backgroundGrid = res[backgroundKey]["gridSize"];
    ref->background.gridSize = {backgroundGrid[0], backgroundGrid[1]};
    ref->background.distance = res[backgroundKey]["distance"];
    nlohmann::json backgroundColorA = res[backgroundKey]["colorA"];
    ref->background.colorA = {backgroundColorA[0], backgroundColorA[1], backgroundColorA[2]};
    nlohmann::json backgroundColorB = res[backgroundKey]["colorB"];
    ref->background.colorB = {backgroundColorB[0], backgroundColorB[1], backgroundColorB[2]};

    for(auto& planet : res["planets"]){
        Planet res = {
            .name = "##", 
            .position = {0,0,0},
            .northVector = {0,0,0},
            .zeroDegree = {0,0,0},
            .radius = 0,
            .mass = 0,
            .ambient = {0,0,0},
            .diffuse = {0,0,0},
            .emission = {0,0,0},
            .luminosity = 0
        };
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
        ref->planets.push_back(res);
    }
}

void serializeAll(std::ostream& outFile, Universe* ref){
    nlohmann::json j;
    j["camera"] = {
        {"position", {ref->camera.position.x, ref->camera.position.y, ref->camera.position.z}},
        {"look", {ref->camera.look.x, ref->camera.look.y, ref->camera.look.z}},
        {"up", {ref->camera.up.x, ref->camera.up.y, ref->camera.up.z}},
        {"fov", ref->camera.verticalFOV}
    };

    j["background"] = {
        {"type", ref->background.type},
        {"gridSize", {ref->background.gridSize.x, ref->background.gridSize.y}},
        {"distance", ref->background.distance},
        {"colorA", {ref->background.colorA.r, ref->background.colorA.g, ref->background.colorA.b}},
        {"colorB", {ref->background.colorB.r, ref->background.colorB.g, ref->background.colorB.b}}
    };

    for(auto& planet : ref->planets){
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
