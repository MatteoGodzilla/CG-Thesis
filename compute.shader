#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) writeonly restrict uniform image2D texOutput;

struct Planet {
    vec3 position;
    vec3 color;
    float radius;
    float mass;
};

struct Ray {
    vec3 pos;
    vec3 dir;
};

layout(std430, binding = 1) readonly buffer transmissionBuffer {
    Planet data[];
};

uniform vec2 viewportSize;
uniform vec3 cameraPos;
uniform vec3 lookDir;
uniform vec3 upVector;
uniform float vFov;

//Orthographic projection
Ray viewportToWorldRay(vec2 viewNorm){
    //In viewport, (0,0) is top left corner
    vec3 right = cross(lookDir, upVector);
    float hFov = vFov * viewportSize.x / viewportSize.y;
    float x = viewNorm.x * 2 - 1;
    float y = (1.0 - viewNorm.y) * 2 - 1;
    //return lookDir + x * hFov * right + y * vFov * upVector;
    return Ray(cameraPos + x * hFov * right + y * vFov * upVector, lookDir);
}

//If a collision exists, returns the closest one (aka lower t value)
//Otherwise it returns -1
float RaySphereIntersection(Ray ray, Planet sphere){
    vec3 K = sphere.position - ray.pos;
    float r = sphere.radius;
    float a = dot(ray.dir, ray.dir);
    float b = -2*dot(ray.dir, K);
    float c = dot(K, K) - r*r;
    float delta = b*b - 4*a*c;
    if(delta >= 0){
        float tPlus = (-b + sqrt(delta)) / (2*a);
        float tMinus = (-b - sqrt(delta)) / (2*a);
        //In theory tMinus should always be the closest?
        //return tMinus;
        return min(tPlus, tMinus);
    } else {
        return -1;
    }
};

void main(){
    vec4 pixel = vec4(0,0,0,0);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 pixelCoordsNorm = vec2(0,0);

    pixelCoordsNorm.x = float(pixelCoords.x)/(gl_NumWorkGroups.x);
    pixelCoordsNorm.y = float(pixelCoords.y)/(gl_NumWorkGroups.y);

    Ray worldRay = viewportToWorldRay(pixelCoordsNorm); 

    float closestT = 9999999999.9;
    vec3 closestColor = vec3(0, 0.6, 0.6);
    for(int i = 0; i < data.length(); i++){
        float t = RaySphereIntersection(worldRay, data[i]);
        if(t >= 0 && t < closestT){
            closestT = t;
            closestColor = data[i].color;
        }
    }
   
    pixel.rgb = closestColor;
    imageStore(texOutput, pixelCoords, pixel);
}
