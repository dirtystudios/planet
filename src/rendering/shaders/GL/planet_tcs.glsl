#version 410 core                                                                 
                                                                                  
layout (vertices = 4) out;                                                        

layout(std140) uniform PlayerViewConstantBuffer {       
    mat4 view;
    mat4 proj;
    mat4 world;
    vec3 eye_pos;
};
        

vec3 GetEdgeMidpoint(vec4 e0, vec4 e1) {
    vec4 p = 0.5f * (e0 + e1);
    return p.xyz;
}

vec3 GetQuadMidPoint(vec4 e0, vec4 e1, vec4 e2, vec4 e3) {
    vec4 p = 0.25f * (e0 + e1 + e2 + e3);    
    return p.xyz;
}

float GetLOD(vec3 p) {
    float min_distance = 1;
    float max_distance = 50;
    float d = distance(p.xyz, eye_pos);
    if(d < 50) {
        return 16;
    } else if(d < 200){
        return 8;
    } else {
        return 1;
    }
    //float s = clamp((d - min_distance) / (max_distance - min_distance), 0.f, 1.f);
    //float power = mix(64.f, 1.f, s);    
    //return power;
}

void main(void) { 
    if(gl_InvocationID == 0) {     
        vec3 mid_point = GetQuadMidPoint(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);                
        gl_TessLevelInner[0] = GetLOD(mid_point);
        gl_TessLevelInner[1] = gl_TessLevelInner[0];        
        gl_TessLevelOuter[0] = GetLOD(GetEdgeMidpoint(gl_in[0].gl_Position, gl_in[2].gl_Position)); // left edge        
        gl_TessLevelOuter[1] = GetLOD(GetEdgeMidpoint(gl_in[0].gl_Position, gl_in[1].gl_Position)); // bottom edge        
        gl_TessLevelOuter[2] = GetLOD(GetEdgeMidpoint(gl_in[1].gl_Position, gl_in[3].gl_Position)); // right edge        
        gl_TessLevelOuter[3] = GetLOD(GetEdgeMidpoint(gl_in[2].gl_Position, gl_in[3].gl_Position)); // top edge
    }
                                                                            
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;     
}                                                                                 
