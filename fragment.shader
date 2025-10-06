#version 430 core

in vec2 uv;
out vec4 outputColor;

uniform sampler2D raytracerOutput;
uniform vec2 raytracerOutputSize;
uniform vec2 viewportSize;

void main(){
    //Pixel perfect scaler
    /*
    vec2 pixel = (uv - vec2(0.5, 0.5)) * viewportSize + raytracerOutputSize / 2;

    if( pixel.x >= 0 && pixel.x < raytracerOutputSize.x && pixel.y >= 0 && pixel.y < raytracerOutputSize.y) {
        outputColor = texture(raytracerOutput, pixel / raytracerOutputSize);
    } else {
        outputColor = vec4(0,0,0,1);
    }
    */

    //Pixel perfect scaler (top left)
    vec2 pixel = uv * viewportSize;
    if( pixel.x >= 0 && pixel.x < raytracerOutputSize.x && pixel.y >= 0 && pixel.y < raytracerOutputSize.y) {
        outputColor = texture(raytracerOutput, pixel / raytracerOutputSize);
    } else {
        outputColor = vec4(0,0,0,1);
    }

    //Fit output into screen
    /*
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
    */
}
