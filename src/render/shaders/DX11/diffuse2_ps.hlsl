cbuffer material : register ( b0 ) {
	float3 Kd;
	float3 Ka;
	float3 Ks;
	float3 Ke;
	float  Ns;
};

Texture2D<float4> tex : register(t0);
SamplerState texSampler : register(s0);

struct PS_INPUT {
	float4 vPosition : SV_POSITION;
	float3 vNormal : NORMAL;
	float2 vTex : TEXCOORD0;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET {    
	float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
	float intensity = max(dot(normalize(Input.vNormal), lightDir), 0.0);
	
	float4 color = tex.Sample(texSampler, Input.vTex);
	
	
	return (color * intensity) + (color * 0.33);
} 