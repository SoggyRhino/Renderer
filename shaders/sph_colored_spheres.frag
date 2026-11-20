#version 460 core

in vec3 vViewPos;
in float vLocalHeight;

uniform mat4 projection;
uniform float uParticleRadius;
uniform float uMinHeight;
uniform float uMaxHeight;

out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    coord.y = -coord.y;

    float r2 = dot(coord, coord);
    if (r2 > 1.0) {
        discard;
    }

    float z = sqrt(1.0 - r2);
    vec3 normal = vec3(coord, z);

    vec3 surfaceViewPos = vViewPos + normal * uParticleRadius;
    vec4 clipPos = projection * vec4(surfaceViewPos, 1.0);
    gl_FragDepth = (clipPos.z / clipPos.w) * 0.5 + 0.5;

    float h = clamp(
        (vLocalHeight - uMinHeight) / max(uMaxHeight - uMinHeight, 0.0001),
        0.0,
        1.0);

    vec3 deepColor = vec3(0.03, 0.28, 0.58);
    vec3 midColor  = vec3(0.14, 0.56, 0.88);
    vec3 peakColor = vec3(0.84, 0.96, 1.00);

    vec3 baseColor = mix(deepColor, midColor, smoothstep(0.0, 0.55, h));
    baseColor = mix(baseColor, peakColor, smoothstep(0.45, 1.0, h));

    FragColor = vec4(baseColor, 1.0);
}
