cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;  
}

cbuffer cbPerObject : register(b1) {
    float4x4 world : WORLD;
}

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