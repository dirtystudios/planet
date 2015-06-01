#version 410 core                                                  
                                                                          
uniform mat4 view_proj;
uniform sampler2D heightmap;
 
layout(location = 0) in vec3 position;                                                  
layout(location = 1) in vec2 tex; 
                                                                                                                              
void main(void) {                                                                  
    gl_Position = view_proj * vec4(position.x, position.y, texture(heightmap, tex).x, 1.f);              
}    
