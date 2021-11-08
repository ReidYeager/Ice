#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 1) uniform sampler2D tex;
layout(set = 1, binding = 2) uniform sampler2D alt;
layout(set = 1, binding = 3) uniform sampler2D three;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 ta = texture(tex, uv);// * vec4(1.0, 0.0, 0.0, 1.0);
  vec4 tb = texture(alt, uv);// * vec4(0.0, 1.0, 0.0, 1.0);
  vec4 tc = texture(three, uv);// * vec4(0.0, 0.0, 1.0, 1.0);

  vec4 color = ta;
  if (uv.x > 0.66)
    color = tc;
  else if (uv.x > 0.33)
    color = tb;

  color = vec4(color.xyzw);

  outColor = color;
}
