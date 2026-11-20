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

inline constexpr std::string_view SIMPLE_2D_FRAG =
    R"__SHADER__(#version 460 core

layout (location = 0) in vec3 vColor;

layout (location = 0) out vec4 FragColor;

void main(){
    FragColor = vec4(vColor, 1.0);
}
)__SHADER__";

inline constexpr std::string_view SIMPLE_2D_VERT =
    R"__SHADER__(#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

layout (location = 0) out vec3 vColor;

uniform mat4 uRotation;

void main(){
    gl_Position = uRotation * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)__SHADER__";

inline constexpr std::string_view SIMPLE_3D_FRAG =
    R"__SHADER__(#version 460 core

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
})__SHADER__";

inline constexpr std::string_view SIMPLE_3D_VERT =
    R"__SHADER__(#version 460 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})__SHADER__";

inline constexpr std::string_view SPH_SIMPLE_FRAG =
    R"__SHADER__(#version 460 core

out vec4 FragColor;

uniform vec3 uColor = vec3(1.0, 1.0, 1.0);

void main() {
    FragColor = vec4(uColor, 1.0);
}
)__SHADER__";

inline constexpr std::string_view SPH_SIMPLE_VERT =
    R"__SHADER__(#version 460 core

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