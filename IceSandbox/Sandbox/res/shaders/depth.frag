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
layout(input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput geoColor;
layout(input_attachment_index = 1, set = 0, binding = 3) uniform subpassInput geoDepth;

layout(location = 0) in vec3 normal;
layout(location = 1) in float viewDot;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 fragLightPos;
layout(location = 4) in vec3 fragViewPos;

layout(location = 0) out vec4 outColor;

#define ambient 0.1

void main() {
    vec3 x = (subpassLoad(geoDepth).xyz - 0.99) * 100;
    outColor = vec4(x.rrr, 1.0);
}
