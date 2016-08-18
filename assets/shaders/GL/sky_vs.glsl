// sky_vertex
#version 410

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;

// constant buffers
layout(std140) uniform _b0_viewConstants {
  	vec3 b0_eyePos;
    mat4 b0_view;
    mat4 b0_proj;  
};

layout(std140) uniform _b1_objectConstants {
    mat4 b1_world;    
};

out VS_OUT {
    vec3 tex_coord0;
} vs_output;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
   vec4 world_pos =(b0_proj * b0_view * b1_world) * vec4(pos, 1.f);
   gl_Position = world_pos.xyww;
   vs_output.tex_coord0 = pos;

}