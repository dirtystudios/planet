// cursor_vertex
#version 410 core

layout (location = 0) in vec3 vertex;

layout(std140) uniform _b1_viewConstants {  	
    mat4 b1_projection;  
    mat4 b1_view;
};

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{
    gl_Position = b1_projection * b1_view * vec4(vertex.xy, 0.0, 1.0);
}