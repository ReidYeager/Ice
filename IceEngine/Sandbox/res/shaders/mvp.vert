#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform MVPMatrices {
	mat4 model;
	mat4 view;
	mat4 proj;
} mvp;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUV;

void main() {
	mat4 pos;
	pos[0] = vec4(1.0, 0.0, 0.0, 0.0);
	pos[1] = vec4(0.0, 1.0, 0.0, 0.0);
	pos[2] = vec4(0.0, 0.0, 1.0, 0.0);
	pos[3] = vec4(gl_InstanceIndex * 2.0, 0, 0.0, 1.0);

	pos[3][1] += (mvp.model * pos)[3][0];

	// gl_Position = mvp.proj * mvp.view * mvp.model * pos * vec4(position, 1.0); // Rotate around world center
	gl_Position = mvp.proj * mvp.view * pos * mvp.model * vec4(position, 1.0); // Rotate around object center
	outNormal = (mvp.model * vec4(normal, 1.0)).xyz;
	outUV = uv;
}

