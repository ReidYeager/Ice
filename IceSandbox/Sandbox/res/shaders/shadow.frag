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

float textureProj(vec4 shadowCoord)
{
	float shadow = 1.0;
	float bias = max(0.0001 * (1.0 - dot(normal, global.dLight.direction)), 0.0001); 
	
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float sampledDepth = texture(shadowMap, shadowCoord.st).r - bias;
		if ( sampledDepth < shadowCoord.z )
		{
			shadow = ambient;
		}
	}
	return shadow;
}

void main() {
    float shadow = textureProj(fragLightPos / fragLightPos.w);

    outColor = vec4(global.dLight.color * shadow, 1.0);
}
