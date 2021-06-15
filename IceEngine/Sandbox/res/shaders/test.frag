#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
	//outColor = vec4(0.5, 0.5, 0.7, 1.0);
	outColor = vec4(normal.xyz, 1.0);
}

//#version 450
//#extension GL_ARB_separate_shader_objects : enable
//
//layout(location = 0) in vec3 fragColor;
//
//layout(location = 0) out vec4 outColor;
//
//void main() {
//    outColor = vec4(fragColor, 1.0);
//}