#pragma once
#include <string_view>

inline constexpr std::string_view MODEL_FRAG = R"__SHADER__(#version 460 core

in vec3 vNormal;
in vec2 vTexCoord;

uniform sampler2D baseColor;
uniform bool hasBaseColor;
uniform vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.6));

out vec4 FragColor;

void main() {
    vec3 albedo = hasBaseColor ? texture(baseColor, vTexCoord).rgb : vec3(0.8);

    float diffuse = max(dot(normalize(vNormal), -lightDir), 0.0);
    vec3 color = albedo * (0.2 + 0.8 * diffuse);

    FragColor = vec4(color, 1.0);
}
)__SHADER__";

inline constexpr std::string_view MODEL_VERT = R"__SHADER__(#version 460 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
)__SHADER__";

inline constexpr std::string_view SIMPLE_2D_FRAG = R"__SHADER__(#version 460 core

layout (location = 0) in vec3 vColor;

layout (location = 0) out vec4 FragColor;

void main(){
    FragColor = vec4(vColor, 1.0);
}
)__SHADER__";

inline constexpr std::string_view SIMPLE_2D_VERT = R"__SHADER__(#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec3 vColor;

uniform mat4 uRotation;

void main(){
    gl_Position = uRotation * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)__SHADER__";

inline constexpr std::string_view SIMPLE_3D_FRAG = R"__SHADER__(#version 460 core

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
})__SHADER__";

inline constexpr std::string_view SIMPLE_3D_VERT = R"__SHADER__(#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})__SHADER__";

inline constexpr std::string_view SPH_COLORED_SPHERES_FRAG = R"__SHADER__(#version 460 core

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
)__SHADER__";

inline constexpr std::string_view SPH_COLORED_SPHERES_VERT = R"__SHADER__(#version 460 core

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
)__SHADER__";

inline constexpr std::string_view SPH_SIMPLE_FRAG = R"__SHADER__(#version 460 core

out vec4 FragColor;

uniform vec3 uColor = vec3(1.0, 1.0, 1.0);

void main() {
    FragColor = vec4(uColor, 1.0);
}
)__SHADER__";

inline constexpr std::string_view SPH_SIMPLE_VERT = R"__SHADER__(#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uPointSize = 8.0;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = uPointSize;
}
)__SHADER__";

