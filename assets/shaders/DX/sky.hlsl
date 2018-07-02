#define Sky_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
	"CBV(b0)," \
	"CBV(b1)," \
	"DescriptorTable(SRV(t0, numDescriptors = 1)), " \
	"StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_LINEAR," \
					   "addressU = TEXTURE_ADDRESS_CLAMP," \
					   "addressV = TEXTURE_ADDRESS_CLAMP," \
					   "addressw = TEXTURE_ADDRESS_CLAMP)"

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

[RootSignature(Sky_RootSig)]
VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
	
	float4x4 wvp = mul(proj, mul(view, world));
	float4 worldPos = mul(wvp, float4(Input.vPos, 1.f));
    output.vPosition = worldPos.xyww;
    output.vTexCoords = Input.vPos;
    return output;
}

[RootSignature(Sky_RootSig)]
float3 PSMain( VS_OUTPUT Input ) : SV_TARGET {  
    return (cubeTexture.Sample(samplerCube, Input.vTexCoords).rgb);
}