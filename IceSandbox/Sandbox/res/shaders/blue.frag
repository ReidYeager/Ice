#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 1) uniform bufferparams {
	vec4 user0;
} params;

layout (set = 1, binding = 2) uniform sampler2D tex;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	// outColor = vec4(normal.xyz, 1.0);
	outColor = texture(tex, uv + params.user0.xx);
}

