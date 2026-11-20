#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uParticleRadius;
uniform float uViewportHeight;

out vec3 vViewPos;
out float vLocalHeight;

void main() {
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    vViewPos = viewPos.xyz;
    vLocalHeight = aPos.y;
    gl_Position = projection * viewPos;

    float dist = -viewPos.z;
    gl_PointSize = (dist > 0.0)
        ? (uViewportHeight * projection[1][1] * uParticleRadius) / dist
        : 0.0;
}
