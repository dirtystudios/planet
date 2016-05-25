#version 410 core                                                  

in vec3 iNormal;

out vec4 outColor;  

void main() {
	vec3 lightDir = normalize(vec3(0, 0, 1));
	float d = max(dot(iNormal, lightDir), 0.1);
    outColor = vec4(d,d,d, 1.f);
}
