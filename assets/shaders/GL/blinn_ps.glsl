// blinn_pixel
#version 410 core                                                  

uniform sampler2D _s0_diffuse;

layout(std140) uniform _b2_blinn {
    vec3 b2_Ka;
    vec3 b2_Kd;
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
    float Ia = 0.05f;
    float Id = 1.f;
    float Is = 1.f;

    vec3 Kd = b2_Kd * texture(_s0_diffuse, i_tex).rgb;
    vec3 Ks = b2_Ks;
    vec3 Ka = b2_Ka;

    float diffuseTerm = max(dot(L, N), 0.0);  
    float specularAngle = clamp(dot(H, N), 0.f, 1.f);
    float specularTerm = b2_Ns == 0.f || specularAngle == 0.f ? 0.f : clamp(pow(specularAngle, b2_Ns), 0.f, 1.f);

    o_color = vec4((Ka * Ia) + (Kd * diffuseTerm * Id) + (Ks * specularTerm * Is), 1.0f);
}
