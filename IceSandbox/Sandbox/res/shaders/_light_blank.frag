#version 450

// 
layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput inPosition;
layout(input_attachment_index = 1, set = 0, binding = 2) uniform subpassInput inNormal;
layout(input_attachment_index = 2, set = 0, binding = 3) uniform subpassInput inAlbedo;
layout(input_attachment_index = 3, set = 0, binding = 4) uniform subpassInput inMaps;
layout(input_attachment_index = 4, set = 0, binding = 5) uniform subpassInput inDepth;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outSwapchain;

void main() {
    outSwapchain = vec4(subpassLoad(inAlbedo).xyz, 1.0);
}
