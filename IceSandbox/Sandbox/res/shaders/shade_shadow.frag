#version 450

// Structs =====

struct DirectionalLight
{
	vec3 direction;
    vec3 color;
    mat4 matrix;
};

// Descriptor inputs =====

layout(set = 0, binding = 0) uniform GlobalInfo {
    mat4 viewProj;
    DirectionalLight dLight;
} global;
layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput inPosition;
layout(input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput inNormal;
layout(input_attachment_index = 2, set = 0, binding = 3) uniform subpassInput inAlbedo;
layout(input_attachment_index = 3, set = 0, binding = 4) uniform subpassInput inMaps;
layout(input_attachment_index = 4, set = 0, binding = 5) uniform subpassInput inDepth;
layout(set = 0, binding = 6) uniform sampler2D shadowMap;

// Vertex output =====

layout(location = 0) out vec4 outSwapchain;

// Code =====

const float ambient = 0.1;
const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

float SampleShadow(vec4 shadowCoord)
{
	float bias = max(0.0001, tan(acos(dot(subpassLoad(inNormal).xyz, global.dLight.direction))));

	float sampledDepth = texture(shadowMap, shadowCoord.xy).r - bias;
	if ( sampledDepth < shadowCoord.z )
	{
		return ambient;
	}

	return 1.0;
}

void main() {
	vec4 fragLightPos = (biasMat * global.dLight.matrix) * vec4(subpassLoad(inPosition).xyz, 1.0);

    float shadow = SampleShadow(fragLightPos / fragLightPos.w);

    outSwapchain = vec4(global.dLight.color * shadow, 1.0);
}
