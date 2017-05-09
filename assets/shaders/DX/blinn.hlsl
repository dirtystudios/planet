cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;  
}

cbuffer cbPerObject : register(b1) {
    float4x4 world : WORLD;
}

cbuffer material : register ( b2 ) {
	float3 Ka;
	float3 Kd;
	float3 Ks;
	float3 Ke;
	float Ns;
};

Texture2D<float4> tex : register(t0);
SamplerState texSampler : register(s0);

struct VS_INPUT {
	float3 vPos : POSITION0;
	float3 vNorm : NORMAL0;
	float2 vTexCoords : TEXCOORD0;
};

struct VS_OUTPUT {
	float3 vNormal : NORMAL0;
	float3 vToCamera : NORMAL1;
	float2 vTex : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
	
	float4 worldPos = mul(world, float4(Input.vPos, 1.f));
	
    output.vPosition = mul(mul(proj, view), worldPos);
	
	output.vToCamera = normalize(eyePos - worldPos.xyz);
	
	output.vNormal = Input.vNorm;
    output.vTex = Input.vTexCoords;
    return output;
}

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {    
    
	float3 L = normalize(float3(1.f, 1.f, 1.f));
	float3 V = normalize(Input.vToCamera);
	float3 N = normalize(Input.vNormal);
	float3 H = normalize(L + V);
	
	float Ia = 0.05;
	float Id = 1.0; 
	float Is = 1.0;
	
	float3 texColor = Kd * tex.Sample(texSampler, Input.vTex).rgb;
	float diffuseTerm = saturate(dot(L, N));
	float specularTerm = saturate(pow(saturate(dot(N, H)), Ns));
	
	float3 ambientLighting = mul(Ka, Ia);
	float3 diffuseColor = mul(mul(texColor, diffuseTerm), Id);
	float3 specLighting = mul(Ks, (specularTerm * Is));
	
	float3 rawr = saturate((ambientLighting + diffuseColor) + specLighting);
	float4 ret = float4(rawr, 1.f);
	return ret;
} 
