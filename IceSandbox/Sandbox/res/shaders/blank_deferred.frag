#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaps;

void main() {
    outPosition = vec4(inPosition, 1.0);
    outNormal = vec4(inNormal, 1.0);
    outAlbedo = vec4(inUV, 0.0, 1.0);
    outMaps = vec4(0.5, 0.5, 0.5, 0.5);
}
