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

#define ambient 0.1

float textureProj(vec4 shadowCoord)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		// I do not know why, but when dist == shadowCoord.z, dist < shadowCoord.z returns true
		// Therefore I multiply dist by 1.01 to avoid this
		float dist = texture( shadowMap, shadowCoord.st).r * 1.01;
		if ( dist < shadowCoord.z && shadowCoord.w > 0.0 ) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

void main() {
    float shadow = textureProj(fragLightPos / fragLightPos.w);

    outColor = vec4(global.dLightColor * shadow, 1.0);
}
