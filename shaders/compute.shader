#version 430 core

struct Planet {
    vec3 position;
    vec3 north;
    vec3 zeroDegree;
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

//Hit 'enum'
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
layout(binding = 3) uniform sampler2DArray planetTextures;
layout(binding = 4) uniform sampler2D planetTextureSize;

uniform vec2 viewportSize;
//Camera
uniform vec3 cameraPos;
uniform vec3 lookDir;
uniform vec3 upVector;
uniform float vFov;
//Background
const int BG_SOLID = 0;
const int BG_GRID = 1;
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

//Returns the t value corresponding to the intersection point between ray and plane
float rayPlaneIntersection(Ray ray, Plane plane){
    vec3 k = ray.pos - plane.origin;
    return -dot(k, plane.normal) / dot(ray.dir, plane.normal);
}

vec3 rayPoint(Ray ray, float t){
    return ray.pos + t * ray.dir;
}

vec3 backgroundGrid(vec3 point, Plane backPlane){
    if(backgroundType == BG_SOLID){
        return backgroundColorA;
    } else {
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

//North vector and zero vector MUST BE NORMALIZED
vec3 spherePointToUV(vec3 point, Planet planet, vec3 northVector, vec3 zeroDegVector){
    vec3 OP = point - planet.position;
    float theta = acos(dot(northVector, OP) / planet.radius);
    vec3 C = northVector * dot(OP, northVector);
    vec3 CP = OP - C;
    float circleRadius = length(CP);
    vec3 scaledZero = zeroDegVector * circleRadius;
    //phi always refers to the smallest angle between CP and scaledZero
    //but in order to map uvs correctly, it has to be adjusted so it goes above PI radians
    float phi = acos( dot(CP, scaledZero) / (circleRadius * circleRadius) ) / 2;
    if(dot(cross(CP, scaledZero), northVector) > 0){
        //it means CP is in the second half of the circle
        phi = 3.1415926 - phi;
    }
    return vec3(phi, theta, circleRadius);
    
}

RayHit castRay(Ray initial, float furthestT){
    RayHit result = RayHit(initial, furthestT, 0, HIT_BACKGROUND, 0, -1);

    int deflections = 0;
    for(int j = 0; j < 10; j++){
        float minT = furthestT;
        uint intersectionType = HIT_BACKGROUND;
        int closestPlanetIndex = -1;

        for(int i = 0; i < data.length(); i++){
            vec3 planeNormal = -lookDir;
            Plane planetLens = Plane(data[i].position, planeNormal); 
            float lensIntersectionT = rayPlaneIntersection(result.ray, planetLens);
            if(lensIntersectionT > 0 && lensIntersectionT <= minT){
                minT = lensIntersectionT;
                intersectionType = HIT_LENS;
                closestPlanetIndex = i;
                //check if the ray also hit the planet
                vec3 intersectionPoint = rayPoint(result.ray, minT);
                float distanceFromCenter = length(intersectionPoint - data[i].position);
                if(distanceFromCenter < data[i].radius){
                    //adjust minT
                    float distanceFromPlane = sqrt(data[i].radius * data[i].radius - distanceFromCenter * distanceFromCenter);
                    vec3 frontPoint = intersectionPoint + planeNormal * distanceFromPlane;
                    vec3 backPoint = intersectionPoint - planeNormal * distanceFromPlane;
                    float frontT = length(frontPoint - result.ray.pos) / length(result.ray.dir);
                    float backT = length(backPoint - result.ray.pos) / length(result.ray.dir);
                    minT = min(frontT, backT);
                    intersectionType = HIT_PLANET;
                }
            }
        }

        vec3 p = rayPoint(result.ray, minT);
        result.distanceTraveled += length(p - result.ray.pos);
        result.t = minT;
        if(intersectionType == HIT_PLANET){
            //shade planet
            result.hitType = HIT_PLANET;
            result.planetIndex = closestPlanetIndex;
            break;
        } else if (intersectionType == HIT_BACKGROUND){
            result.hitType = HIT_BACKGROUND;
            break;
        } else {
            result.ray = bendRay(result.ray, data[closestPlanetIndex], p);  
            result.hitType = HIT_LENS;
            result.deflections++;
        }
    }
    return result;
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
    float backPlaneT = rayPlaneIntersection(worldRay, backPlane);
    RayHit firstHit = castRay(worldRay, backPlaneT * 2);  

    if(firstHit.hitType == HIT_PLANET){
        //shade planet
        Planet hit = data[firstHit.planetIndex];
        vec3 hitPoint = rayPoint(firstHit.ray, firstHit.t);
        vec3 zeroDegVector = normalize(hit.zeroDegree);
        vec3 northVector = normalize(hit.north);
        vec3 uv = spherePointToUV(hitPoint, hit, northVector, zeroDegVector);
        //debug.rgb = uv; 
        vec2 uvScale = texture(planetTextureSize, vec2(0, float(firstHit.planetIndex) / data.length())).rg;
        debug.rg = uvScale; 
        vec2 uvNormalized = uv.xy / 3.1415926;
        //debug.rg = uvNormalized;
        vec4 texColor = texture(planetTextures, vec3(uvNormalized.x * uvScale.x, uvNormalized.y * uvScale.y, firstHit.planetIndex));
        float totalFactor = 0;
        for(int i = 0; i < data.length(); i++){
            if(data[i].luminosity > 0 && i != firstHit.planetIndex){
                //Point-light luminosity
                vec3 hitPoint = rayPoint(firstHit.ray, firstHit.t);
                vec3 towardsPlanet = normalize(data[i].position - hitPoint); 
                vec3 normal = normalize(hitPoint - hit.position);
                vec3 color = data[i].emission * hit.diffuse;
                float distance = length(hitPoint - data[i].position); // we assume it's a point light for this example, when in reality it isn't
                float factor = data[i].luminosity / (4 * 3.1415926 * distance * distance); 
                totalFactor += factor;
            }
        }
        pixel.rgb = mix(hit.ambient, texColor.rgb * min(totalFactor,1), texColor.a);
    } else if (firstHit.hitType == HIT_BACKGROUND){
        vec3 p = rayPoint(firstHit.ray, firstHit.t);
        pixel.rgb = backgroundGrid(p, backPlane);
    } 

    imageStore(texOutput, pixelCoords, pixel);
    imageStore(debugOutput, pixelCoords, debug);
}

