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

// layout(set = 1, binding = 0) uniform MaterialUniform {
//     float test;
// } material;

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
    // vec4 modelPosition = model.transform * vec4(inPosition, 1.0);
    gl_Position = global.viewProj * vec4(inPosition, 1.0);

    // outNormal = normalize(mat3(model.transform) * inNormal);
    outNormal = inNormal;
    outUV = inUV;
}