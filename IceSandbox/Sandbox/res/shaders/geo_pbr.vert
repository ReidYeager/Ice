#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

// layout(set = 0, binding = 0) uniform GlobalUniform {
// } global;

layout(set = 1, binding = 0) uniform CameraUniform {
    mat4 viewProj;
    // DirectionalLight dLight;
} camera;

// layout(set = 2, binding = 0) uniform MaterialUniform {
// } material;

layout(set = 3, binding = 0) uniform ModelUniform {
    mat4 transform;
} model;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;

void main() {
    vec4 worldPosition = model.transform * vec4(inPosition, 1.0);

    gl_Position = camera.viewProj * worldPosition;

    outPosition = worldPosition.xyz;
    outNormal = normalize(mat3(model.transform) * inNormal);
    outUV = inUV;
}