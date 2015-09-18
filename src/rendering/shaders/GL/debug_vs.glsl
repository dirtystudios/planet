#version 410 core                                                  
                                                                           
in vec3 position;  

uniform mat4 proj_view;                                                
                                                                   
                                                                   
void main(void) {                                                                  
    gl_Position = proj_view * vec4(position, 1.f);              
}                                                                  
