#version 430 core

in vec2 uv;
out vec4 outputColor;

uniform sampler2D raytracerOutput;
uniform vec2 raytracerOutputSize;
uniform vec2 viewportSize;

void main(){
    //Pixel perfect scaler
    vec2 pixel = (uv - vec2(0.5, 0.5)) * viewportSize + raytracerOutputSize / 2;

    if( pixel.x >= 0 && pixel.x < raytracerOutputSize.x && pixel.y >= 0 && pixel.y < raytracerOutputSize.y) {
        outputColor = texture(raytracerOutput, pixel / raytracerOutputSize);
    } else {
        outputColor = vec4(0,0,0,1);
    }
}
