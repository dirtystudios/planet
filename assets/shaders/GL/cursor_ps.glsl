// cursor_pixel
#version 410 core                                                  

layout(std140) uniform _b2_textConstants {  	
    vec3 b2_textColor;
};

out vec4 color;  

void main() {
    color = vec4(b2_textColor, 1.0);
}
