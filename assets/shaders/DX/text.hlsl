#define Sky_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
	"CBV(b1)," \
	"CBV(b2)," \
	"DescriptorTable(SRV(t0, numDescriptors = 1)), " \
	"StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_LINEAR," \
					   "addressU = TEXTURE_ADDRESS_CLAMP," \
					   "addressV = TEXTURE_ADDRESS_CLAMP," \
					   "addressw = TEXTURE_ADDRESS_CLAMP)"

cbuffer cbProj : register(b1) {
    float4x4 projection : PROJECTION;
    float4x4 viewCbConstant;
}

cbuffer cbPerObject : register(b2) {
    float3 textColor : COLOR0;
}

Texture2D<float> text : register(t0);
SamplerState textSampler : register(s0);

struct VS_INPUT {
	float3 vPos : POSITION0;
	float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT {
	float2 vTexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

[RootSignature(Sky_RootSig)]
VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(mul(projection, viewCbConstant), float4(Input.vPos.xyz, 1.0));
    output.vTexCoords = Input.vTex;
    return output;
}

[RootSignature(Sky_RootSig)]
float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {  

	float alpha = text.Sample(textSampler, Input.vTexCoords);
	float4 color = float4(textColor, alpha);
    return color;
}