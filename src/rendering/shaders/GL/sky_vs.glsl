 #version 410                               
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
uniform mat4 world;
out vec3 tex_coord0;
out gl_PerVertex {
  vec4 gl_Position;
};
void main() {
   vec4 world_pos = world * vec4(pos, 1.f);
   gl_Position = world_pos.xyww;
   tex_coord0 = pos;

}