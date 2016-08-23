cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;  
}

cbuffer cbPerObject : register(b1) {
    float4x4 world : WORLD;
}

TextureCube<float4> cubeTexture : register(t0);
SamplerState samplerCube : register(s0);

struct VS_INPUT {
	float3 vPos : POSITION;
};

struct VS_OUTPUT {
	float3 vTexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
	
	float4x4 wvp = mul(proj, mul(view, world));
	float4 worldPos = mul(wvp, float4(Input.vPos, 1.f));
    output.vPosition = worldPos.xyww;
    output.vTexCoords = Input.vPos;
    return output;
}

float3 PSMain( VS_OUTPUT Input ) : SV_TARGET {  
    return (cubeTexture.Sample(samplerCube, Input.vTexCoords).rgb);
}