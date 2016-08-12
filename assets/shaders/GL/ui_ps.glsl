#version 410 core


// constant buffers
layout(std140) uniform _b2_frameConstants {  	
    vec4 b2_bgColor;  
    vec4 b2_borderColor;
    vec2 b2_borderSize;
};

layout (location = 0) in vec2 i_texCoords;

out vec4 color;

void main() {
    //becomes positive when inside the border and 0 when outside
    vec2 within_border = clamp((i_texCoords * i_texCoords - i_texCoords) - (b2_borderSize * b2_borderSize - b2_borderSize), 0, 1); 
    color = (i_texCoords.x < b2_borderSize.x && i_texCoords.y < b2_borderSize.y)? b2_bgColor : b2_borderColor;//(-within_border.x == within_border.y) ?  b2_bgColor : b2_borderColor;
}