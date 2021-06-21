#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(tex, uv);
  outColor = color * vec4(normal.xyz, 1.0);
}
