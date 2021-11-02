#version 450
#extension GL_ARB_separate_shader_objects : enable

// Uses the entire 128 bytes of guaranteed space for push constants
layout(push_constant) uniform Push {
	mat4 model;
	mat4 viewProj;
} push;

layout(set = 0, binding = 0) uniform bufferparams {
	vec4 user0;
	vec4 user1;
} params;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec4 user0;

void main() {
	gl_Position = push.viewProj * push.model * vec4(position.x, position.y, position.z, 1.0);
	outNormal = (push.model * vec4(normal, 1.0)).xyz;
	outUV = uv;
	user0 = params.user0;
}

