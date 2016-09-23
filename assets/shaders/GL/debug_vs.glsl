// debug_vertex
#version 410 core

layout (location = 0) in vec2 vertex;

layout(std140) uniform _b1_viewConstants {  	
    mat4 b1_projection;  
};

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{
    gl_Position = b1_projection * vec4(vertex, 0.0, 1.0);
}