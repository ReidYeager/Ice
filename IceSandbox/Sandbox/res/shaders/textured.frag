#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 2) in vec2 uv;

layout(set = 1, binding = 0) uniform sampler2D image;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(image, uv);
}
