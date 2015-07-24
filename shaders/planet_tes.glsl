#version 410 core                                                                 
                                                                                  
layout (quads, fractional_odd_spacing, ccw) in;                                   
              
layout(std140) uniform PlayerViewConstantBuffer {       
    mat4 view;
    mat4 proj;
    mat4 world;
    vec3 eye_pos;
};                                                                                                                                      
                                                                                  
void main(void) {                                                                                 
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);       
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);       
    vec4 pos = mix(p1, p2, gl_TessCoord.y);

    float x_squared = pos.x * pos.x;
    float y_squared = pos.y * pos.y;
    float z_squared = pos.z * pos.z;

    vec4 mapping;
    mapping.x = pos.x * sqrt(1.f - (y_squared / 2.f) - (z_squared / 2.f) + (y_squared * z_squared / 3.f));
    mapping.y = pos.y * sqrt(1.f - (z_squared / 2.f) - (x_squared / 2.f) + (z_squared * x_squared / 3.f));
    mapping.z = pos.z * sqrt(1.f - (x_squared / 2.f) - (y_squared / 2.f) + (x_squared * y_squared / 3.f));
    
    gl_Position = proj * view * world * pos; 
}  
