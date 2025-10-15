#version 430 core

struct Planet {
    vec3 position;
    float radius;
    float mass;
    vec3 ambient;
    vec3 diffuse;
    vec3 emission;
    float luminosity;
};

struct Ray {
    vec3 pos;
    vec3 dir;
};

struct Plane {
    vec3 origin;
    vec3 normal;
};

const uint HIT_BACKGROUND = 0;
const uint HIT_PLANET = 1;
const uint HIT_LENS = 2;

struct RayHit {
    Ray ray;
    float t;
    float distanceTraveled;
    uint hitType;
    int deflections;
    int planetIndex;
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
uniform vec3 backgroundColorA;
uniform vec3 backgroundColorB;

//Orthographic projection
Ray viewportToWorldRay(vec2 viewNorm){
    //In viewport, (0,0) is top left corner
    vec3 right = cross(lookDir, upVector);
    float hFov = vFov * viewportSize.x / viewportSize.y;
    float x = viewNorm.x * 2 - 1;
    float y = (1.0 - viewNorm.y) * 2 - 1;
    return Ray(cameraPos + x * hFov * right + y * vFov * upVector, normalize(lookDir));
}

//Returns the t values associated with the collision
//Otherwise it returns vec2(-1, -1)
//We assume that the ray has a direction vector that is normalized
vec2 RaySphereIntersection(Ray ray, Planet sphere){
    vec3 K = sphere.position - ray.pos;
    float r = sphere.radius;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0 * dot(ray.dir, K);
    float c = dot(K, K) - r*r;
    float delta = b*b - 4*a*c;
    if(delta >= 0){
        float tPlus = (-b + sqrt(delta)) / (2*a);
        float tMinus = (-b - sqrt(delta)) / (2*a);
        return vec2(tMinus, tPlus);
    } else {
        return vec2(-1, -1);
    }
};

//Returns the t value corresponding to the intersection point between ray and plane
float RayPlaneIntersection(Ray ray, Plane plane){
    vec3 k = ray.pos - plane.origin;
    return -dot(k, plane.normal) / dot(ray.dir, plane.normal);
}

//Returns the t value of the point where the ray is closest to the center of the sphere
//WE ASSUME THAT THE DIRECTION OF THE RAY IS NORMALIZED 
float RaySphereClosestPoint(Ray ray, Planet sphere){
    return dot(ray.dir, sphere.position - ray.pos);
}

vec3 rayPoint(Ray ray, float t){
    return ray.pos + t * ray.dir;
}

vec3 backgroundGrid(vec3 point, Plane backPlane){
    vec3 planeUp = upVector; 
    vec3 planeRight = cross(upVector, backPlane.normal);
    vec3 K = point - backPlane.origin;
    float planeX = dot(K, planeRight) / length(planeRight);
    float planeY = dot(K, planeUp) / length(planeUp);
    float resX = mod(planeX, backgroundGridSize.x) / backgroundGridSize.x;
    float resY = mod(planeY, backgroundGridSize.y) / backgroundGridSize.y;

    const float gridThickness = 0.1;
    vec3 result = backgroundColorA;
    if(resX < gridThickness || resY < gridThickness){
        result = backgroundColorB;
    }
    return result;
}

Ray bendRay(Ray original, Planet p, vec3 closestPoint){
    float impactRadius = length(closestPoint - p.position);
    const float C = 299792458.0; 
    const float G = 6.67430e-11;
    float alpha = 4 * G * p.mass / (C * C * impactRadius);
    //float alpha = p.mass / impactRadius;
    float deviation = length(original.dir) * tan(alpha);
    vec3 towardsCenter = normalize(p.position - closestPoint);
    vec3 newDir = normalize(original.dir + towardsCenter * deviation);
    return Ray(closestPoint, newDir);
}

//There has to be a better way to do this
float getClosestT(vec2 tVals){
    if(tVals.x > 0){
        if(tVals.y > 0){
            return min(tVals.x, tVals.y);
        } else {
            return tVals.x;
        }
    } else {
        if(tVals.y > 0){
            return tVals.y;
        } else {
            return -1;
        }
    }
}

void main(){
    vec4 pixel = vec4(0,0,0,0);
    vec4 debug = vec4(-1,0,0,0);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 pixelCoordsNorm = vec2(0,0);

    pixelCoordsNorm.x = float(pixelCoords.x)/(gl_NumWorkGroups.x);
    pixelCoordsNorm.y = float(pixelCoords.y)/(gl_NumWorkGroups.y);

    Ray worldRay = viewportToWorldRay(pixelCoordsNorm); 
    Plane backPlane = Plane(cameraPos + lookDir * backgroundDistance, -lookDir);

    int deflections = 0;
    for(int j = 0; j < 10; j++){
        float minT = RayPlaneIntersection(worldRay, backPlane);
        uint intersectionType = HIT_BACKGROUND;
        int closestPlanetIndex = -1;

        for(int i = 0; i < data.length(); i++){
            vec3 planeNormal = -lookDir;
            Plane planetLens = Plane(data[i].position, planeNormal); 
            float lensIntersectionT = RayPlaneIntersection(worldRay, planetLens);
            if(lensIntersectionT > 0 && lensIntersectionT < minT){
                minT = lensIntersectionT;
                intersectionType = HIT_LENS;
                closestPlanetIndex = i;
                //check if the ray also hit the planet
                vec3 intersectionPoint = rayPoint(worldRay, minT);
                float distanceFromCenter = length(intersectionPoint - data[i].position);
                if(distanceFromCenter < data[i].radius){
                    //adjust minT
                    float distanceFromPlane = sqrt(data[i].radius * data[i].radius - distanceFromCenter * distanceFromCenter);
                    vec3 frontPoint = intersectionPoint + planeNormal * distanceFromPlane;
                    vec3 backPoint = intersectionPoint - planeNormal * distanceFromPlane;
                    float frontT = length(frontPoint - worldRay.pos) / length(worldRay.dir);
                    float backT = length(backPoint - worldRay.pos) / length(worldRay.dir);
                    minT = min(frontT, backT);
                    intersectionType = HIT_PLANET;
                }
            }
        }
        
        vec3 p = rayPoint(worldRay, minT);
        debug.r = closestPlanetIndex;
        debug.g = intersectionType;
        debug.b = minT;

        if(intersectionType == HIT_PLANET){
            //shade planet
            pixel.rgb = data[closestPlanetIndex].diffuse;
            break;
        } else if (intersectionType == HIT_BACKGROUND){
            pixel.rgb = backgroundGrid(p, backPlane);
            break;
        } else {
            worldRay = bendRay(worldRay, data[closestPlanetIndex], p);  
            deflections++;
        }
    }
   
    debug.rgb = worldRay.dir;

    imageStore(texOutput, pixelCoords, pixel);
    imageStore(debugOutput, pixelCoords, debug);
}
