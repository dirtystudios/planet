// debug_pixel
#version 410 core                                                  

layout (location = 0) in vec2 i_tex;
layout (location = 1) in vec3 i_col;

out vec4 color;  

void main() {
    color = vec4(i_col, 1.0);
}
