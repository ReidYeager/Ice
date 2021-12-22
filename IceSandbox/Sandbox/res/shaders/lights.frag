#version 450

struct DirectionalLight {
    vec4 direction;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalInfo {
    mat4 viewProj;
    DirectionalLight dLight;
} global;

layout(location = 0) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 c = global.dLight.color * (dot(normal, global.dLight.direction.xyz) * 0.5 + 0.5);
    outColor = c;
}
