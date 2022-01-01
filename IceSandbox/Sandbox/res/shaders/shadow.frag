#version 450

layout(set = 0, binding = 0) uniform GlobalInfo {
    mat4 viewProj;
    vec3 dLightDirection;
    vec3 dLightColor;
    mat4 dLightMatrix;
} global;

layout(set = 0, binding = 1) uniform sampler2D shadowMap;

layout(location = 0) in vec3 normal;
layout(location = 1) in float viewDot;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 fragLightPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 projectCoords = fragLightPos.xyz / fragLightPos.w;

    // TODO : Figure out why these hacks (kind of) work.

    // Remapping the z value causes all currentDepth values to be > shadowDepth
    projectCoords.xy = projectCoords.xy * 0.5 + 0.5;

    float shadowDepth = texture(shadowMap, projectCoords.xy).r;
    float currentDepth = projectCoords.z;

    // Using the raw current depth, anything not in shadow results in a spiraling pattern
    float shadow = (currentDepth * 0.99) < shadowDepth ? 1.0 : 0.0;

    outColor = vec4(vec3(shadow), 1.0);
}
