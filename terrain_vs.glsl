#version 410 core                                                  
                  
                  /*                                                        
uniform mat4 view_proj;
uniform mat4 world;
uniform sampler2D heightmap;
 
layout(location = 0) in vec3 position;                                                  
layout(location = 1) in vec2 tex; 
                                                                                                                              
void main(void) {                                                                  
    gl_Position = view_proj * world * vec4(position.x, position.y, texture(heightmap, tex).x, 1.f);              
}    

*/

                                                                         
uniform mat4 view;
uniform mat4 proj;
uniform mat4 world;
uniform sampler2DArray heightmap_tile_array;
//uniform sampler2D heightmap;
uniform int tile_index;
 
layout(location = 0) in vec2 position;                                                  
layout(location = 1) in vec2 tex; 
                                                                                                                              
void main(void) {       
    int t = tile_index;        
    vec4 pos = world * vec4(position.x, position.y, texture(heightmap_tile_array, vec3(tex, float(tile_index))).x, 1.f);              
    float x_squared = pos.x * pos.x;
    float y_squared = pos.y * pos.y;
    float z_squared = pos.z * pos.z;

    vec4 mapping = pos;    
    //mapping.x = pos.x * sqrt(1000.f - (y_squared / 2.f) - (z_squared / 2.f) + (y_squared * z_squared / 3.f));
    //mapping.y = pos.y * sqrt(1000.f - (z_squared / 2.f) - (x_squared / 2.f) + (z_squared * x_squared / 3.f));
    //mapping.z = pos.z * sqrt(1000.f - (x_squared / 2.f) - (y_squared / 2.f) + (x_squared * y_squared / 3.f));

    gl_Position = proj * view * mapping;
}    

