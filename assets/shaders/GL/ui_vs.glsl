// ui_vertex
#version 410 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in vec2 texcoord;


// constant buffers
layout(std140) uniform _b1_viewConstants {  	
    mat4 b1_projection;  
};

// constant buffers
layout(std140) uniform _b2_frameConstants {  	
    vec4 b2_bgColor;  
    vec4 b2_borderColor;
    vec2 b2_borderSize;
    vec3 b2_position;
};

layout (location = 0) out vec2 o_texCoords;

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{
    gl_Position = b1_projection * vec4(vertex.xyz + b2_position, 1.0);
    o_texCoords = texcoord;
}