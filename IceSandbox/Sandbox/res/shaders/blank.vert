#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProj;
} global;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out float viewDot;

void main() {
    gl_Position = global.viewProj * vec4(position, 1.0);
    fragColor = normal;

    vec3 camPos = vec3(global.viewProj[0][3], global.viewProj[1][3], global.viewProj[2][3]);
    viewDot = dot(-camPos, normal);
}