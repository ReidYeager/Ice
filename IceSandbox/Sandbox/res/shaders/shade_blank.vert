#version 450

struct VertexStruct {
    vec3 position;
    vec2 uv;
};

const VertexStruct verts[6] = {
    { vec3( 1.0, -1.0, 0.0), vec2(1.0, 0.0) }, // TR
    { vec3(-1.0, -1.0, 0.0), vec2(0.0, 0.0) }, // TL
    { vec3(-1.0,  1.0, 0.0), vec2(0.0, 1.0) }, // BL

    { vec3(-1.0,  1.0, 0.0), vec2(0.0, 1.0) }, // BL
    { vec3( 1.0,  1.0, 0.0), vec2(1.0, 1.0) }, // BR
    { vec3( 1.0, -1.0, 0.0), vec2(1.0, 0.0) }, // TR
};

// Vertex output
layout(location = 0) out vec2 outUV;

void main() {
    // Render screen-filling quad
    gl_Position = vec4(verts[gl_VertexIndex].position, 1.0);
    outUV = verts[gl_VertexIndex].uv;
}