#version 410 core

// attribute
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_norm;
layout (location = 2) in vec2 a_tex;

// constant buffers
layout(std140) uniform _b0_viewConstants {
    vec3 b0_eyePos;
    mat4 b0_view;
    mat4 b0_proj;
};

layout(std140) uniform _b1_objectConstants {
    mat4 b1_world;    
};

// output
layout (location = 0) out vec3 o_normal;
layout (location = 1) out vec3 o_toCamera;
layout (location = 2) out vec2 o_tex;

out gl_PerVertex {
    vec4 gl_Position;
};

////////////////////////////////////////////////////////

void main() {	
    vec4 worldPosition = b1_world * vec4(a_pos, 1.0);

    o_tex = a_tex;
    o_normal = a_norm; //normalize((invWV * vec4(norm, 1.0)).xyz);
    o_toCamera = normalize(b0_eyePos - worldPosition.xyz);	

    gl_Position = (b0_proj * b0_view) * worldPosition;
}