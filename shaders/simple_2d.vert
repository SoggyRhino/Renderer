#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec3 vColor;

uniform mat4 uRotation;

void main(){
    gl_Position = uRotation * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
