#version 410 core                                                  
                                                                          
layout(std140) uniform PlayerViewConstantBuffer {       
    mat4 view;
    mat4 proj;
    mat4 world;
    vec3 eye_pos;
};
 
in vec3 position;                                                  
                                                                                                                               
void main(void) {                                                                  
    gl_Position = proj * view * world * vec4(position, 1.f);              
}                                                                  
