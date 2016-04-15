 #version 410                               
in vec3 tex_coord0;
uniform samplerCube baseTexture;
out vec4 color;
void main() {
   color = texture(baseTexture, tex_coord0);
}