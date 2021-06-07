#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform sampler2D alt;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 A = texture(tex, uv);
	vec4 B = texture(alt, uv);
	float x = round(uv.x);
	outColor = (A * x) + (B * (1 - x));
}

