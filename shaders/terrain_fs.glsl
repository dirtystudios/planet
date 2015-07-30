#version 410 core                                                  

uniform sampler2DArray heightmap_normals_tile_array;
uniform int normals_tile_index;


in vec2 t;
in vec3 c;                                                                     
out vec4 color;                                                    
                                                                 

void main(void) {     
    vec3 l = normalize(vec3(0, 0, 1));  
    vec3 normal = texture(heightmap_normals_tile_array, vec3(t, float(normals_tile_index))).xyz;
    
    //color = vec4(normal, 1.f);
    

    float d = clamp(dot(l, c), 0.f, 1.f);
    color = vec4(d, d, d, 1.f);
} 
