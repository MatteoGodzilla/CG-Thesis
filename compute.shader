#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) writeonly restrict uniform image2D texOutput;

struct Planet {
    vec3 position;
    vec3 color;
    float radius;
    float mass;
};

layout(std430, binding = 1) readonly buffer transmissionBuffer {
    Planet data[];
};

void main(){
    vec4 pixel = vec4(0,0,0,0);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 pixelCoordsNorm = vec2(0,0);

    pixelCoordsNorm.x = float(pixelCoords.x)/(gl_NumWorkGroups.x);
    pixelCoordsNorm.y = float(pixelCoords.y)/(gl_NumWorkGroups.y);

    //TODO: pass these informations as a matrix uniform
    //Orthographic projection
    const float LEFT = -5; //meters
    const float RIGHT = 5; //meters
    const float UP = 5; //meters
    const float DOWN = -5; //meters

    vec3 worldRay = vec3(0,0,0); 
    worldRay.x = LEFT + pixelCoordsNorm.x * (RIGHT - LEFT);
    worldRay.y = UP + pixelCoordsNorm.y * (DOWN - UP); //because y axis is flipped by opengl

    vec2 delta = worldRay.xy - data[0].position.xy;
    //pixel.rg = abs(delta);

    //TESTING, DOES NOT WORK IN 3D
    if(delta.x * delta.x + delta.y * delta.y <= data[0].radius * data[0].radius){
        pixel.rgb = data[0].color;
    }

    //Convert pixel norm coordinates to world rays

    /*
    pixel.r = pixelCoordsNorm.x;
    pixel.g = pixelCoordsNorm.y;
    */

    /*
    for(int i = 0; i < data.length(); i+= 5){
        float xi = data[i + 0];
        float yi = data[i + 1];
        float ri = data[i + 2];
        float gi = data[i + 3];
        float bi = data[i + 4];

        pixel.r += mix(ri, 0, distance(pixelCoordsNorm, vec2(xi, yi)) * 2);
        pixel.g += mix(gi, 0, distance(pixelCoordsNorm, vec2(xi, yi)) * 2);
        pixel.b += mix(bi, 0, distance(pixelCoordsNorm, vec2(xi, yi)) * 2);
    }
    */
    
    /*
    pixel.r = 1 - pixel.r;
    pixel.g = 1 - pixel.g;
    pixel.b = 1 - pixel.b;
    */

    imageStore(texOutput, pixelCoords, pixel);
}
