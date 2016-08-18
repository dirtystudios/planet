// sky_pixel
 #version 410                               
in vec3 tex_coord0;
uniform samplerCube _s0_baseTexture;
out vec4 color;
void main() {
   color = texture(_s0_baseTexture, tex_coord0);
}