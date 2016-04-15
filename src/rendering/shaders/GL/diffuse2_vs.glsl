#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

out vec3 oNormal;

uniform mat4 wvp;
uniform mat4 invWV;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
	oNormal = normalize((invWV * vec4(norm, 1.0)).xyz);
    gl_Position = wvp * vec4(pos.xyz, 1.0);
}