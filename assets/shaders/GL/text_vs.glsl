// text_vertex
#version 410 core
layout (location = 0) in vec3 pos; // <vec2 pos, vec2 tex>
layout (location = 1) in vec2 tex;
out vec2 TexCoords;

// constant buffers
layout(std140) uniform _b1_viewConstants {  	
    mat4 b1_projection;  
    mat4 b1_view;
};

out gl_PerVertex {
  vec4 gl_Position;
};

void main()
{
    gl_Position = b1_projection * b1_view * vec4(pos, 1.0);
    TexCoords = tex;
}