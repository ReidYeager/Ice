#version 450

// Structs =====

struct DirectionalLight
{
	vec3 direction;
    vec3 color;
    mat4 matrix;
};

// Descriptor inputs =====

layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProj;
    // DirectionalLight dLight;
} global;

// layout(set = 2, binding = 0) uniform ModelUniform {
//     mat4 transform;
// } model;

// Vertex input =====

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

// Vertex output =====

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

// Code =====

void main() {
    mat4 vp;
    vp *= 0;
    vp[3][3] = 1.0;

    vp[0][0] = 1.0;
    vp[1][1] = 1.0;
    vp[2][2] = 1.0;

    gl_Position = vp * vec4(inPosition, 1.0);

    outNormal = inNormal;
    outUV = inUV;

    // vec4 modelPosition = model.transform * vec4(inPosition, 1.0);

    // gl_Position = global.viewProj * modelPosition;
    // outNormal = normalize(mat3(model.transform) * inNormal);
    // outUV = inUV;
}