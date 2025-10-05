#include "serialization.h"

void deserializeAll(std::istream& inFile, Camera* camera, Background* background, std::vector<Planet>* ref){
    std::string temp;
    getline(inFile,temp); //Camera 
    inFile >> camera->position.x >> camera->position.y >> camera->position.z;
    inFile >> camera->look.x >> camera->look.y >> camera->look.z;
    inFile >> camera->up.x >> camera->up.y >> camera->up.z;
    inFile >> camera->verticalFOV;
    getline(inFile, temp); //consume the newline after FOV
    getline(inFile, temp); //consume the empty line afterwards 

    getline(inFile, temp); // Background
    inFile >> background->gridSize.x >> background->gridSize.y;
    inFile >> background->distance;
    getline(inFile, temp); //consume the newline after distance
    getline(inFile, temp); //consume the empty line afterwards 

    getline(inFile, temp); //get the number of planets
    int count = std::stoi(temp);
    for(int i = 0; i < count; i++){
        Planet res = {"##", {0,0,0}, {0,0,0}, 0, 0};
        getline(inFile, res.name);
        inFile >> res.position.x >> res.position.y >> res.position.z;
        inFile >> res.color.x >> res.color.y >> res.color.z;
        inFile >> res.radius;
        inFile >> res.mass;
        //std::cout << "====" << std::endl;
        //std::cout << res.name << std::endl;
        ref->push_back(res);
        getline(inFile, temp); //consume the newline after the radius
        getline(inFile, temp); //consume the empty line afterwards
    }
}

void serializeAll(std::ostream& outFile, Camera* camera, Background* background, std::vector<Planet>* ref){
    outFile << "Camera" << std::endl;
    outFile << camera->position.x << " " << camera->position.y << " " << camera->position.z << std::endl;
    outFile << camera->look.x << " " << camera->look.y << " " << camera->look.z << std::endl;
    outFile << camera->up.x << " " << camera->up.y << " " << camera->up.z << std::endl;
    outFile << camera->verticalFOV << std::endl;
    outFile << std::endl;
    outFile << "Background" << std::endl;
    outFile << background->gridSize.x << "  " << background->gridSize.y << std::endl; 
    outFile << background->distance << std::endl; 
    outFile << std::endl;
    outFile << ref->size() << std::endl;
    for(auto& p : *ref){
        outFile << p.name << std::endl;
        outFile << p.position.x << " " << p.position.y << " " <<  p.position.z << std::endl;
        outFile << p.color.x << " " <<  p.color.y << " " <<  p.color.z << std::endl;
        outFile << p.radius << std::endl;
        outFile << p.mass << std::endl;
        outFile << std::endl;
    }
}
