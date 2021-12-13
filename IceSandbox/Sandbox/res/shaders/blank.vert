#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProj;
    vec4 dLightDirection;
    vec4 dLightColor;
    mat4 dLightMatrix;
} global;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out float viewDot;
layout(location = 2) out vec2 uvOut;
layout(location = 3) out vec4 fragLightPos;

void main() {
    vec4 modelPosition = vec4(position, 1.0);

    gl_Position = global.viewProj * modelPosition;
    outNormal = normal;

    vec3 camPos = vec3(global.viewProj[0][3], global.viewProj[1][3], global.viewProj[2][3]);
    viewDot = dot(-camPos, normal);
    uvOut = uv;
    fragLightPos = global.dLightMatrix * modelPosition;
}