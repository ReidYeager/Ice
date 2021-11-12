#version 450
#extension GL_ARB_separate_shader_objects : enable

// Uses the entire 128 bytes of guaranteed space for push constants
layout(push_constant) uniform Push {
	mat4 model;
	mat4 viewProj;
} push;

layout(set = 0, binding = 0) uniform globalParams {
	mat4 vp;
} global;

layout(set = 1, binding = 0) uniform bufferparams {
	float x;
	float y;
	float z;
	float w;
} params;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main() {
	gl_Position = global.vp * push.model * vec4(position.x, position.y + params.y, position.z, 1.0);
	outNormal = (push.model * vec4(normal, 1.0)).xyz;
	outUV = uv;
}

