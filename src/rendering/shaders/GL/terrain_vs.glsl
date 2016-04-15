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

uniform float rradius;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 world;
uniform mat4 trans;
uniform mat4 scale;
uniform int elevations_tile_index;
uniform int normals_tile_index;
uniform sampler2DArray base_texture;
uniform sampler2DArray normal_texture;


layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex;

layout(location = 3) out vec3 Normal;

out gl_PerVertex {
  vec4 gl_Position;
};


void main(void) {
    float height = 0.f * texture(base_texture, vec3(tex, float(elevations_tile_index))).x;
    vec3 normal = texture(normal_texture, vec3(tex, float(normals_tile_index))).xyz;
    


    vec4 pos = trans * scale * vec4(position.x, position.y, rradius / 2.f, 1.f);    
    vec3 mapped = normalize(vec3(pos.x, pos.y, pos.z)) * (height + (rradius / 2.f));

    vec4 a = world*pos;
    // vec3 mapped = vec3(position.x, position.y, 0);


    // pos.x = height * (pos.x / rradius);
    // pos.y = height * (pos.y / rradius);
    // pos.z = height * (1.f / rradius);
    // pos.w = sqrt(position.x * position.x + position.y * position.y + 1);

    Normal = normal;  
    float rr = rradius;  

    /*
    // in order to sphereicalize the cube, need to be -1 > x > 1, -1 > y > 1, -1 > z > 1
    float radius = rradius;
    vec4 mapping = pos;

    float x_squared = pos.x * pos.x;
    float y_squared = pos.y * pos.y;
    float z_squared = pos.z * pos.z;
    mapping.x = radius * pos.x * sqrt(1.f - (y_squared / 2.f) - (z_squared / 2.f) + (y_squared * z_squared / 3.f));
    mapping.y = radius * pos.y * sqrt(1.f - (z_squared / 2.f) - (x_squared / 2.f) + (z_squared * x_squared / 3.f));
    mapping.z = (radius + height) * pos.z * sqrt(1.f - (x_squared / 2.f) - (y_squared / 2.f) + (x_squared * y_squared / 3.f));
    */

    gl_Position = proj * view  * world *  vec4(mapped.x, mapped.y, mapped.z, 1.f);
}
