#version 430 core

in vec2 uv;
out vec4 outputColor;

uniform sampler2D raytracerOutput;
uniform vec2 raytracerOutputSize;
uniform vec2 viewportSize;

void main(){
    //Pixel perfect scaler
    vec2 pixel = uv * viewportSize;
    if(pixel.x <= raytracerOutputSize.x && pixel.y <= raytracerOutputSize.y){
        outputColor = texture(raytracerOutput, pixel / raytracerOutputSize);
    } else {
        outputColor = vec4(0,0,0,1);
    }
}
