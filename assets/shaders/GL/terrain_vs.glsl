// terrain_vertex
#version 410 core

uniform sampler2DArray _s0_heightmap;

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex;

layout(std140) uniform _b0_viewConstants {
    vec3 b0_eyePos;
    mat4 b0_view;
    mat4 b0_proj;
};

layout(std140) uniform _b1_objectConstants {
    mat4 b1_world;  
    uint b1_heightmapIndex;
    uint b1_lod;  
};

layout (location = 0) out vec2 o_tex;
layout (location = 1) out vec3 o_norm;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    float height = texture(_s0_heightmap, vec3(a_tex, b1_heightmapIndex), 0).x * 10.f;

    vec4 worldPos = b1_world * vec4(a_pos, 1.f);
    vec3 p = normalize(worldPos.xyz) * (height + 256.f);

    worldPos = vec4(p, 1.f);

    o_tex = a_tex;
    o_norm = a_norm;
    gl_Position = b0_proj * b0_view * worldPos;

}