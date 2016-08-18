// ui_vertex
#version 410 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>


// constant buffers
layout(std140) uniform _b1_viewConstants {  	
    mat4 b1_projection;  
};


layout (location = 0) out vec2 o_texCoords;

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{
    gl_Position = b1_projection * vec4(vertex.xy, 0.0, 1.0);
    o_texCoords = vertex.zw;
}