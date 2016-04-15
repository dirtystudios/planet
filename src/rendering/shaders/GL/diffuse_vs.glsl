#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;

out vec2 texCoord;

uniform mat4 wvp;

out gl_PerVertex {
  vec4 gl_Position;
};

void main() {
	texCoord = tex;
    gl_Position = wvp * vec4(pos.xy, 0.0, 1.0);
}