#version 450

layout(set = 0, binding = 0) uniform GlobalInfo {
    mat4 viewProj;
    vec4 dLightDirection;
    vec4 dLightColor;
} global;

layout(location = 0) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 c = vec3(1, 1, 1) * (dot(normal, global.dLightDirection.xyz) * 0.5);
    outColor = vec4(c, 1.0);
}
