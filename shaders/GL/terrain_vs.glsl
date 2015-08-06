#version 400 core                                                  

uniform mat4 view;
uniform mat4 proj;
uniform mat4 world;
uniform int elevations_tile_index;
uniform int normals_tile_index;
uniform sampler2DArray heightmap_elevations_tile_array;
uniform sampler2DArray heightmap_normals_tile_array;

 
layout(location = 0) in vec2 position;                                                  
layout(location = 1) in vec2 tex; 

out vec2 t;
out vec3 c;
                                                                                                                              
void main(void) {           
    float height = 250.f * texture(heightmap_elevations_tile_array, vec3(tex, float(elevations_tile_index))).x;
    vec3 normal = texture(heightmap_normals_tile_array, vec3(tex, float(normals_tile_index))).xyz;
    vec4 pos = vec4(position.x, position.y, height, 1.f);                  
    c = normal;
    t = tex;



    /*
    // in order to sphereicalize the cube, need to be -1 > x > 1, -1 > y > 1, -1 > z > 1
    float radius = 500;
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

