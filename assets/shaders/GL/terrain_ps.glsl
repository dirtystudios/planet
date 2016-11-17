// terrain_pixel
#version 410 core

uniform sampler2DArray _s0_heightmap;

layout(std140) uniform _b1_objectConstants {
    mat4 b1_world;  
    uint b1_heightmapIndex;
    uint b1_lod;  
};

layout (location = 0) in vec2 i_tex;
layout (location = 1) in vec3 i_norm;

out vec4 o_color;

vec4 getColor(uint index) {
    if (index > 64) {
        return vec4(1.f, 1.f, 1.f, 1.f);
    }

    const uint colors[64] = uint[](
        uint(0x0000FF), uint(0xFFFF00), uint(0x1CE6FF), uint(0xFF34FF), uint(0xFF4A46), uint(0x008941), uint(0x006FA6), uint(0xA30059), uint(0xFFDBE5), uint(0x7A4900), uint(0x0000A6), uint(0x63FFAC), uint(0xB79762), uint(0x004D43), uint(0x8FB0FF), uint(0x997D87),
        uint(0x5A0007), uint(0x809693), uint(0xFEFFE6), uint(0x1B4400), uint(0x4FC601), uint(0x3B5DFF), uint(0x4A3B53), uint(0xFF2F80), uint(0x61615A), uint(0xBA0900), uint(0x6B7900), uint(0x00C2A0), uint(0xFFAA92), uint(0xFF90C9), uint(0xB903AA), uint(0xD16100),
        uint(0xDDEFFF), uint(0x000035), uint(0x7B4F4B), uint(0xA1C299), uint(0x300018), uint(0x0AA6D8), uint(0x013349), uint(0x00846F), uint(0x372101), uint(0xFFB500), uint(0xC2FFED), uint(0xA079BF), uint(0xCC0744), uint(0xC0B9B2), uint(0xC2FF99), uint(0x001E09),
        uint(0x00489C), uint(0x6F0062), uint(0x0CBD66), uint(0xEEC3FF), uint(0x456D75), uint(0xB77B68), uint(0x7A87A1), uint(0x788D66), uint(0x885578), uint(0xFAD09F), uint(0xFF8A9A), uint(0xD157A0), uint(0xBEC459), uint(0x456648), uint(0x0086ED), uint(0x886F4C)
    );

    uint  mask = ~(((1 << 16) - 1) << 8);
    float r    = (colors[index] >> 16 & mask) / 255.f;
    float g    = (colors[index] >> 8 & mask) / 255.f;
    float b    = (colors[index] & mask) / 255.f;
    return vec4(r, g, b, 1.f);
}

void main() {
    float height = texture(_s0_heightmap, vec3(i_tex, b1_heightmapIndex)).x;
    o_color      = (vec4((height + 1.f) / 2.f)) * getColor(b1_lod);
}
