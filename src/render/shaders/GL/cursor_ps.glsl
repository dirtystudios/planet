#version 410 core                                                  

uniform vec3 textColor;

out vec4 color;  

void main() {
    color = vec4(textColor, 1.0);
}
