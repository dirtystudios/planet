#version 410 core

/*
layout(std140) uniform PerViewBuffer {
    mat4 view;
    mat4 proj;
    vec3 eye_pos;
};

layout(std140) uniform PerTerrainBuffer {
    mat4 world;
    int elevations_tile_index;
    int normals_tile_index;
};
*/

uniform mat4 view;
uniform mat4 proj;
uniform mat4 world;
uniform int elevations_tile_index;
uniform int normals_tile_index;
uniform sampler2DArray base_texture;
uniform sampler2DArray normal_texture;


layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex;

layout(location = 2) out vec2 t;
layout(location = 3) out vec3 c;

out gl_PerVertex {
  vec4 gl_Position;
};

void main(void) {
    float height = 250.f * texture(base_texture, vec3(tex, float(elevations_tile_index))).x;
    vec3 normal = texture(normal_texture, vec3(tex, float(normals_tile_index))).xyz;
    vec4 pos = vec4(position.x, position.y, height, 1.f);
    c = normal;
    t = tex;

    /*
    // in order to sphereicalize the cube, need to be -1 > x > 1, -1 > y > 1, -1 > z > 1
    float radius = 500
    vec4 mapping = pos;

    float x_squared = pos.x * pos.x;
    float y_squared = pos.y * pos.y;
    float z_squared = pos.z * pos.z;
    mapping.x = radius * pos.x * sqrt(1.f - (y_squared / 2.f) - (z_squared / 2.f) + (y_squared * z_squared / 3.f));
    mapping.y = radius * pos.y * sqrt(1.f - (z_squared / 2.f) - (x_squared / 2.f) + (z_squared * x_squared / 3.f));
    mapping.z = (radius + height) * pos.z * sqrt(1.f - (x_squared / 2.f) - (y_squared / 2.f) + (x_squared * y_squared / 3.f));
    */

    gl_Position = proj * view * world * pos;
}
