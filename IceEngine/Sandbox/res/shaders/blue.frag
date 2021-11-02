#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform bufferparams {
	vec4 user0;
} params;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4((params.user0.x + 1.0) / 2.0, 0.0, 1.0, 1.0);
}

