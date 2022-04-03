#version 450

layout(set = 1, binding = 0) uniform sampler2D texAlbedo;
layout(set = 1, binding = 1) uniform sampler2D texNormal;
layout(set = 1, binding = 2) uniform sampler2D texMetallic;
layout(set = 1, binding = 3) uniform sampler2D texRoughness;

layout(set = 2, binding = 0) uniform ModelUniform {
    mat4 transform;
} model;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaps;

vec3 TransformNormalMapToModel()
{
	vec3 tangetNormal = texture(texNormal, inUV).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(inPosition);
	vec3 q2 = dFdy(inPosition);
	vec2 st1 = dFdx(inUV);
	vec2 st2 = dFdy(inUV);

	vec3 N = normalize(inNormal);
	// vec3 T = normalize(q1 * st2.x - q2 * st1.x);
	vec3 T = normalize(q1 * st2.y - q2 * st1.y);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangetNormal);
	// return normalize(tangetNormal);
}

void main() {
    outPosition = vec4(inPosition, 1.0);

    vec3 N = TransformNormalMapToModel();
    outNormal = vec4(N, 1.0);

    vec3 col = texture(texAlbedo, inUV).xyz;

    outAlbedo = vec4(col, 1.0);
    outMaps = vec4(texture(texMetallic, inUV).x, texture(texRoughness, inUV).x, inUV.xy);
}
