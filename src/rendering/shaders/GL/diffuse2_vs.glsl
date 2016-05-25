#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;


layout(std140) uniform _slot0_viewConstants {
  	vec3 eye_pos;
    mat4 view;
    mat4 proj;  
};

layout(std140) uniform _slot1_objectConstants {
    mat4 world;    
};

out vec3 oNormal;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {	
	oNormal = norm; //normalize((invWV * vec4(norm, 1.0)).xyz);
    gl_Position = (proj * view * world) * vec4(pos.xyz, 1.0);
}