#version 410 core                                                  

layout(location = 3) in vec3 Normal;                                                          
out vec4 color;                                                    
                                                                 

void main(void) {     
    vec3 l = normalize(vec3(0, 0, 1));      
    color = vec4(Normal, 1.f);
    

    //float d = clamp(dot(l, c), 0.f, 1.f);
    //color = vec4(d, d, d, 1.f);
} 
