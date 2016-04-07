 #version 410                               
in vec3 tex_coord0;
uniform samplerCube base_texture;
out vec4 color;
void main() {
   color = texture(base_texture, tex_coord0);
}