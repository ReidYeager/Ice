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
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(position, 1.0);
//	gl_Position = vec4(position, 1.0);
	outNormal = (mvp.model * vec4(normal, 1.0)).xyz;
	outUV = uv;
}

//#version 450
//#extension GL_ARB_separate_shader_objects : enable
//
//layout(binding = 0) uniform MVPMatrices {
//  mat4 model;
//  mat4 view;
//  mat4 proj;
//} mvp;
//
//layout(location = 0) out vec3 fragColor;
//
//vec2 positions[3] = vec2[](
//  vec2(0.0, -0.5),
//  vec2(0.5, 0.5),
//  vec2(-0.5, 0.5)
//);
//
//vec3 colors[3] = vec3[](
//  vec3(1.0, 0.0, 0.0),
//  vec3(0.0, 1.0, 0.0),
//  vec3(0.0, 0.0, 1.0)
//);
//
//void main() {
//  gl_Position = mvp.proj * mvp.view * mvp.model * vec4(positions[gl_VertexIndex], 0.0, 1.0);
//  fragColor = colors[gl_VertexIndex];
//}
