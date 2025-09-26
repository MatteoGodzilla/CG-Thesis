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

struct Plane {
    vec3 origin;
    vec3 normal;
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
    return Ray(cameraPos + x * hFov * right + y * vFov * upVector, normalize(lookDir));
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

float RayPlaneIntersection(Ray ray, Plane plane){
    vec3 k = ray.pos - plane.origin;
    return -dot(k, plane.normal) / dot(ray.dir, plane.normal);
}

//Returns the t value of the point where the ray is closest to the center of the sphere
float RaySphereClosestPoint(Ray ray, Planet sphere){
    //WE ASSUME THAT THE DIRECTION OF THE RAY IS NORMALIZED 
    return dot(ray.dir, sphere.position - ray.pos);
}

vec3 rayPoint(Ray ray, float t){
    return ray.pos + t * ray.dir;
}

vec3 background(vec3 point, Plane backPlane){
    vec3 planeUp = upVector; 
    vec3 planeRight = cross(upVector, backPlane.normal);
    vec3 K = point - backPlane.origin;
    float planeX = dot(K, planeRight) / length(planeRight);
    float planeY = dot(K, planeUp) / length(planeUp);
    float resX = abs(planeX - int(planeX)); //[0,1)
    float resY = abs(planeY - int(planeY)); //[0,1)
    const float gridSize = 0.1;
    vec3 result = vec3(0, 0.2, 0.2);
    if(resX < gridSize || resY < gridSize){
        result = vec3(0,0,0);
    }
    return result;
}

Ray bendRay(Ray original, Planet p, vec3 closestPoint){
    float impactRadius = length(closestPoint - p.position);
    float alpha = p.mass / (50 * impactRadius);
    float deviation = length(original.dir) * tan(alpha);
    vec3 towardsCenter = p.position - closestPoint;
    vec3 newDir = normalize(original.dir + towardsCenter * deviation);
    return Ray(closestPoint, newDir);
}

void main(){
    vec4 pixel = vec4(0,0,0,0);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 pixelCoordsNorm = vec2(0,0);

    pixelCoordsNorm.x = float(pixelCoords.x)/(gl_NumWorkGroups.x);
    pixelCoordsNorm.y = float(pixelCoords.y)/(gl_NumWorkGroups.y);

    Ray worldRay = viewportToWorldRay(pixelCoordsNorm); 

    float closestDistance2 = 9999999999.9;
    int closestPlanetI = -1;
    vec3 closestPoint = vec3(0,0,0);
    for(int i = 0; i < data.length(); i++){
        float t = RaySphereClosestPoint(worldRay, data[i]);
        vec3 P = rayPoint(worldRay, t);
        vec3 K = P - data[i].position;
        if(t >= 0 && dot(K,K) < closestDistance2){
            closestDistance2 = dot(K,K);
            closestPlanetI = i; 
            closestPoint = P;
        }
    }

    float radius2 = data[closestPlanetI].radius * data[closestPlanetI].radius;
    if(closestDistance2 < radius2){
        //We have an intersection
        pixel.rgb = data[closestPlanetI].color;
    } else {
        Plane backPlane = Plane(cameraPos + lookDir * 100, -lookDir);
        //We have missed the planet -> draw atmosphere
        vec3 atmosphere = vec3(1,1,1) - data[closestPlanetI].color;
        float delta = closestDistance2 - radius2;
        Ray bent = bendRay(worldRay, data[closestPlanetI], closestPoint);

        float backT = RayPlaneIntersection(bent, backPlane);
        vec3 backPoint = rayPoint(bent, backT);
        vec3 backgroundColor = background(backPoint, backPlane);
        //pixel.rgb = mix(backgroundColor, atmosphere, clamp((1 - delta / 50),0,1));
        pixel.rgb = backgroundColor;
    }
    
    imageStore(texOutput, pixelCoords, pixel);
}
