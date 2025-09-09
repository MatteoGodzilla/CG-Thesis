#version 430 core

in vec2 uv;
out vec4 outputColor;

uniform sampler2D raytracerOutput;

void main(){
    outputColor = texture(raytracerOutput, uv);
}
