#version 430 core

in vec2 uv;
out vec4 outputColor;

uniform sampler2D raytracerOutput;
uniform vec2 raytracerOutputSize;
uniform vec2 viewportSize;

void main(){
    float outputRatio = raytracerOutputSize.x / raytracerOutputSize.y;
    float width = min(viewportSize.x, viewportSize.y * outputRatio);    
    float height = min(viewportSize.y, viewportSize.x / outputRatio);    

    vec2 uvCopy = uv * viewportSize ;
    vec2 spaceRemaining = viewportSize - vec2(width, height);
    uvCopy -= spaceRemaining / 2;
    uvCopy /= vec2(width, height);

    if(uvCopy.x < 0 || uvCopy.x > 1 || uvCopy.y < 0 || uvCopy.y > 1){
        outputColor = vec4(0,0,0,0);
    } else {
        outputColor = texture(raytracerOutput, uvCopy);
    }
}
