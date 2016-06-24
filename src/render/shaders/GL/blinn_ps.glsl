#version 410 core                                                  

uniform sampler2D _s0_diffuse;

layout(std140) uniform _b2_blinn {
	vec3 b2_Kd;	
	vec3 b2_Ka;
	vec3 b2_Ks;
	vec3 b2_Ke;
	float b2_Ns;
};

layout (location = 0) in vec3 i_normal;
layout (location = 1) in vec3 i_toCamera;
layout (location = 2) in vec2 i_tex;

out vec4 o_color;  

////////////////////////////////////////////////////////


void main() {
	// normalize vectors after interpolation
   	vec3 L = normalize(vec3(1, 1, 1));
   	vec3 V = normalize(i_toCamera);
   	vec3 N = normalize(i_normal);
   	vec3 H = normalize(L + V);   

   	// fake light parameters
   	float Ia = 0.05;
    float Id = 1.0;
    float Is = 1.0;

   	vec3 Kd = texture(_s0_diffuse, i_tex).rgb;
   	vec3 Ks = b2_Ks;
   	vec3 Ka = b2_Ka;
  
    float diffuseTerm = max(dot(L, N), 0.0);    
	float specularTerm = pow(max(dot(N, H), 0.0), b2_Ns);
    
    o_color = vec4((Ka*Ia) + (Kd * diffuseTerm * Id) + (Ks * specularTerm * Is), 1.0f);
}
