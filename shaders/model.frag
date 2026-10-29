#version 460 core

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
