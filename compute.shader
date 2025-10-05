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
    //return vec3(0,0,0);
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
                if(isInFront > 0.5){
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
    } else {
        //we assume the ray has gone into the background
      
        float backT = RayPlaneIntersection(worldRay, backPlane);
        vec3 backPoint = rayPoint(worldRay, backT);
        vec3 backgroundColor = background(backPoint, backPlane);
        //pixel.rgb = mix(backgroundColor, atmosphere, clamp((1 - delta / 50),0,1));
        pixel.rgb = backgroundColor;
        
        //pixel.rgb = vec3(1,1,1) * float(bounces) / 10;
        
        //pixel.rgb = abs(worldRay.dir);
    } /*else {
        //pixel.rgb = vec3(1,1,1) * float(j) / 10;
        pixel.rgb = worldRay.dir + vec3(0.5, 0.5, 0.5);
    }
    */

    /*
    vec2 intersection = RaySphereIntersection(worldRay, data[0]);
    pixel.rg = intersection;
    */

    /*
    vec3 planetColor = data[0].color;
    float maxT = 999999999.9;
    float closestPlanetT = 999999999.9;
    closestPlanetT = RaySphereIntersection(worldRay, data[0]);
    /*
    for(int i = 0; i < 1; i++){
        float t = RaySphereIntersection(worldRay, data[i]);
        closestPlanetT = t;
        planetColor = data[i].color;
        if(t < closestPlanetT){
            hitPlanet = true;
            planetIndex = i;
            closestPlanetT = t;
            planetColor = data[i].color;
        }
    }
    */
    /*
    if(closestPlanetT > 0){
        pixel.rgb = planetColor;
    } else {
        pixel.rgb = vec3(0, 0.2, 0.2);
    }
    */
    

    //Stop conditions:
    // Hit of a planet
    // Hit of the back plane
    // Number of deflections

    /*
    bool hitPlanet = false;
    int planetIndex = 0;
    vec3 planetColor = vec3(0,0,0);
    bool hitBackground = false;
    /*
    for(int j = 0; !hitPlanet && !hitBackground && j < 10; j++){
        //
    }
    */

    /*
    if(hitPlanet){
        pixel.rgb = data[planetIndex].color;
    } else if(hitBackground){
        //
        pixel.rgb = vec3(0, 0.2, 0.2); 
    } else {
        pixel.rgb = vec3(1, 0, 1);
    }
    */

    /*
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
    */
    
    imageStore(texOutput, pixelCoords, pixel);
}
