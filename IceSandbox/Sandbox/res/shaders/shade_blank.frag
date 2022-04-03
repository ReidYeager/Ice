#version 450

layout(set = 0, binding = 1, input_attachment_index = 0) uniform subpassInput inPosition;
layout(set = 0, binding = 2, input_attachment_index = 1) uniform subpassInput inNormal;
layout(set = 0, binding = 3, input_attachment_index = 2) uniform subpassInput inAlbedo;
layout(set = 0, binding = 4, input_attachment_index = 3) uniform subpassInput inMaps;
layout(set = 0, binding = 5, input_attachment_index = 4) uniform subpassInput inDepth;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outSwapchain;

void main() {
    // outSwapchain = vec4(subpassLoad(inAlbedo).rgb, 1.0);
    outSwapchain = vec4(1.0, 0.0, 0.0, 1.0);
}
