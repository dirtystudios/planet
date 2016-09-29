// debug_vertex
#version 410 core

layout (location = 0) in vec3 i_pos;
layout (location = 1) in vec2 i_tex;
layout (location = 2) in vec3 i_col;

layout(std140) uniform _b0_viewConstants {
    mat4 b1_view;  	
    mat4 b1_projection;  
};

// output
layout (location = 0) out vec2 o_tex;
layout (location = 1) out vec3 o_col;

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{ 
    gl_Position = b1_projection * b1_view *  vec4(i_pos, 1.0);
    o_tex = i_tex;
    o_col = i_col;
}