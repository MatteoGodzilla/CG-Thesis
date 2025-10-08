#version 430 core

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

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) writeonly restrict uniform image2D texOutput;
layout(rgba32f, binding = 1) writeonly restrict uniform image2D debugOutput;
layout(std430, binding = 2) readonly buffer transmissionBuffer {
    Planet data[];
};

uniform vec2 viewportSize;
uniform vec3 cameraPos;
uniform vec3 lookDir;
uniform vec3 upVector;
uniform float vFov;
uniform vec2 gridSize;
uniform float backgroundDistance;

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
    vec3 K = ray.pos - sphere.position;
    float r = sphere.radius;
    //using reduced quadratic
    float a = 1; //dot(ray.dir, ray.dir);
    float b = dot(ray.dir, K);
    float c = dot(K, K) - r*r;
    float delta = b*b - a*c;
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

vec3 background(vec3 point, Plane backPlane){
    /*
    vec3 planeUp = upVector; 
    vec3 planeRight = cross(upVector, backPlane.normal);
    vec3 K = point - backPlane.origin;
    float planeX = dot(K, planeRight) / length(planeRight);
    float planeY = dot(K, planeUp) / length(planeUp);
    float resX = mod(planeX, gridSize.x) / gridSize.x;
    float resY = mod(planeY, gridSize.y) / gridSize.y;

    const float gridThickness = 0.1;
    vec3 result = vec3(0, 0.2, 0.2);
    if(resX < gridThickness || resY < gridThickness){
        result = vec3(0,0,0);
    }
    return result;
    //return vec3(resX, resY, 0);
    */
    return vec3(0,0.1,0.1);
}

Ray bendRay(Ray original, Planet p, vec3 closestPoint){
    const float C = 299792458.0; 
    const float G = 6.67430e-11;
    float impactRadius = length(closestPoint - p.position);
    float alpha = 4 * G * p.mass / (C * C * impactRadius);
    //float alpha = p.mass / impactRadius;
    float deviation = length(original.dir) * tan(alpha);
    vec3 towardsCenter = p.position - closestPoint;
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
   
    bool hitPlanet = false;
    int planetIndex = -1;
    Plane backPlane = Plane(cameraPos + lookDir * backgroundDistance, -lookDir);
    int closestMissedPlanet = -1;
    int j;
    int bounces = 0;

    for(j = 0; j < 10 && !hitPlanet; j++){
        float closestHit = 999999999.9;
        float closestMissT = 999999999.9;
        closestMissedPlanet = -1;
        for(int i = 0; i < data.length(); i++){
            vec2 tVals = RaySphereIntersection(worldRay, data[i]);
            float t = getClosestT(tVals);
            if(t > 0 && t < closestHit){
                //We hit a planet!
                hitPlanet = true;
                planetIndex = i;
                closestHit = t;
            } else if(t < 0){
                Plane planetLens = Plane(data[i].position, -lookDir); 
                float impactT = RayPlaneIntersection(worldRay, planetLens);
                if(impactT > 0 && impactT < closestMissT){
                    closestMissedPlanet = i;
                    closestMissT = impactT;
                }
                /*
                //We didn't hit a planet by definition
                // Check if the ray should be bent or go straight to the background
                float isInFront = dot(worldRay.dir, data[i].position - worldRay.pos);
                if(isInFront > 0.5){compute
                    float impactT = RaySphereClosestPoint(worldRay, data[i]);
                    if(impactT > 0 && impactT < closestMissT){
                        //We should bend the ray 
                        closestMissedPlanet = i;
                        closestMissT = impactT;
                    }
                }
                */
            }
        }
        if(closestMissedPlanet >= 0){
            //Apply angle
            vec3 closestPoint = rayPoint(worldRay, closestMissT);
            worldRay = bendRay(worldRay, data[closestMissedPlanet], closestPoint);
            bounces++;
        } else {
            break;
        }
    }

    if(hitPlanet){
        pixel.rgb = data[planetIndex].color * (1 - float(bounces) / 10);
        debug.r = planetIndex;
    } else {
        //we assume the ray has gone into the background
      
        float backT = RayPlaneIntersection(worldRay, backPlane);
        vec3 backPoint = rayPoint(worldRay, backT);
        vec3 backgroundColor = background(backPoint, backPlane);
        //pixel.rgb = mix(backgroundColor, atmosphere, clamp((1 - delta / 50),0,1));
        pixel.rgb = backgroundColor;
        
        //pixel.rgb = vec3(1,1,1) * float(bounces) / 10;
        
        //pixel.rgb = abs(worldRay.dir);
    } 
    //debug.rgb = vec3(1,1,1) * float(bounces) / 10;
    debug.g = bounces;
    
    imageStore(texOutput, pixelCoords, pixel);
    imageStore(debugOutput, pixelCoords, debug);
}
