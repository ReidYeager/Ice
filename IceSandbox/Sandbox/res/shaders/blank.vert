#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProj;
} global;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = global.viewProj * vec4(position, 1.0);
    fragColor = normal;
}