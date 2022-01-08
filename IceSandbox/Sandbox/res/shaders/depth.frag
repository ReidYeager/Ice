#version 450

struct DirectionalLight
{
	vec3 direction;
    vec3 color;
    mat4 matrix;
};

layout(set = 0, binding = 0) uniform GlobalInfo {
    mat4 viewProj;
    DirectionalLight dLight;
} global;

layout(set = 0, binding = 1) uniform sampler2D shadowMap;
layout(set = 0, binding = 2) uniform sampler2D depthTexture;

layout(location = 0) in vec3 normal;
layout(location = 1) in float viewDot;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 fragLightPos;
layout(location = 4) in vec3 fragViewPos;

layout(location = 0) out vec4 outColor;

#define ambient 0.1

void main() {
    outColor = vec4(texture(depthTexture, fragViewPos.xy).rrr, 1.0);
}
