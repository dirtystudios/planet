 #version 410                               
in vec3 tex_coord0;
uniform samplerCube cube_texture;
out vec4 color;
void main() {
   color = texture(cube_texture, tex_coord0);
}