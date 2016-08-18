// text_pixel
#version 410 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D _s0_base_texture;

// constant buffers
layout(std140) uniform _b2_textConstants {  	
    vec3 b2_textColor;
};


void main()
{    
    
    float alpha = texture(_s0_base_texture, TexCoords).r;
    color = vec4(b2_textColor, alpha);
}  