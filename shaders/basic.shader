#version 430 core

struct Planet {
    vec3 position;
    vec3 color;
    float radius;
    float mass;
};

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) writeonly restrict uniform image2D texOutput;
layout(rgba32f, binding = 1) writeonly restrict uniform image2D debugOutput;
layout(std430, binding = 2) readonly buffer transmissionBuffer {
    Planet data[];
};

uniform vec2 viewportSize;
//Camera
uniform vec3 cameraPos;
uniform vec3 lookDir;
uniform vec3 upVector;
uniform float vFov;
//Background
uniform int backgroundType;
uniform vec2 backgroundGridSize;
uniform float backgroundDistance;
uniform vec2 backgroundColorA;
uniform vec2 backgroundColorB;

void main(){
    vec4 pixel = vec4(0,0,0,0);
    vec4 debug = vec4(-1,0,0,0);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    imageStore(texOutput, pixelCoords, pixel);
    imageStore(debugOutput, pixelCoords, debug);
}
