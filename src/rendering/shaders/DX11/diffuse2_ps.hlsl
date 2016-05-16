
struct PS_INPUT {
	float4 vPosition : SV_POSITION;
	float3 vNormal : NORMAL;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET {    
	float3 lightDir = normalize(float3(0, 0, 1));
	float d = max(dot(Input.vNormal, lightDir), 0.1);
	
	return float4(d, d, d, 1.f);
} 