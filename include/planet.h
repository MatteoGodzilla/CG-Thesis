#pragma once

struct Planet {
    alignas(16) float position[3]; //x,y,z, in meters
    alignas(16) float color[3]; //r,g,b
    float radius; //in meters
    float mass; //in kilograms
};
