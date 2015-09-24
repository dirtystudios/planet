#version 410 core

uniform vec4 bgColor;
uniform vec4 borderColor;
uniform vec2 borderSize;

in vec2 vTexCoords;
in vec4 vPosition;

out vec4 color;

void main() {
    //becomes positive when inside the border and 0 when outside
    vec2 within_border = clamp((vTexCoords * vTexCoords - vTexCoords) - (borderSize * borderSize - borderSize), 0, 1); 
    color = (-within_border.x == within_border.y) ?  bgColor : borderColor;
}