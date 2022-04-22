#version 450

// Descriptor inputs =====

// layout(set = 0, binding = 0) uniform GlobalUniform {
// } global;

layout(set = 1, binding = 0) uniform CameraUniform {
    mat4 viewProj;
    // DirectionalLight dLight;
} camera;

layout(set = 2, binding = 0) uniform MaterialUniform {
    vec4 test;
} material;

layout(set = 3, binding = 0) uniform ModelUniform {
    mat4 transform;
} model;

// Fragment input =====

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

// Fragment output =====

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4((material.test.xyz * 0.5) + 0.5, 1.0);
    //outColor = vec4(inNormal, 1.0);
}
