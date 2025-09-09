#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D texOutput;

layout(std430, binding = 1) readonly buffer transmissionBuffer {
    //Multiple elements in the form:
    //x normalized, y normalized, r, g, b
    float data[];
};

void main(){
    vec4 pixel = vec4(0,0,0,0);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 pixelCoordsNorm = vec2(0,0);

    pixelCoordsNorm.x = float(pixelCoords.x)/(gl_NumWorkGroups.x);
    pixelCoordsNorm.y = float(pixelCoords.y)/(gl_NumWorkGroups.y);
    
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

    imageStore(texOutput, pixelCoords, pixel);
}
