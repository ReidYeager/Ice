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

layout(location = 0) in vec3 normal;
layout(location = 1) in float viewDot;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec4 fragLightPos;

layout(location = 0) out vec4 outColor;

#define ambient 0.1

float SampleShadow(vec4 shadowCoord)
{
	float bias = max(0.0001, tan(acos(dot(normal, global.dLight.direction)))); // Maintains a bit of acne at the light/shadow transition
	// float bias = tan(acos(dot(normal, global.dLight.direction))); // Removes all acne on self-shadows (dot(normal, lightDir) < 0) but also removes the casted shadow on other objects

	float sampledDepth = texture(shadowMap, shadowCoord.xy).r - bias;
	if ( sampledDepth < shadowCoord.z )
	{
		return ambient;
	}

	return 1.0;
}

void main() {
    float shadow = SampleShadow(fragLightPos / fragLightPos.w);

    outColor = vec4(global.dLight.color * shadow, 1.0);
}
