#define UI_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
	"CBV(b1)," \
	"CBV(b2),"

cbuffer cbConstant : register(b1) {
    float4x4 projection : PROJECTION;
    float4x4 viewCbConstant;
}

cbuffer cbPerObject : register(b2) {
    float4 bgColor : COLOR0;
    float4 borderColor : COLOR1;
    float2 borderSize;
    float3 position;
}

struct VS_INPUT {
	float4 vPos : POSITION0;
    float2 vTexCoords : TEXCOORD0;
};

struct VS_OUTPUT {
	float2 vTexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

[RootSignature(UI_RootSig)]
VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
	/*float4x4 tmp = mul (projection, rotation);
    output.vPosition = mul(tmp, Input.vPos);
    output.vTexCoords = Input.vTex;*/
    output.vPosition = mul(mul(projection, viewCbConstant), float4(position + Input.vPos.xyz, 1.0));
	output.vTexCoords = Input.vTexCoords;
    return output;
}

[RootSignature(UI_RootSig)]
float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {
    float2 within_border = saturate(
                            (Input.vTexCoords * Input.vTexCoords - Input.vTexCoords)
                            - (borderSize * borderSize - borderSize)); //becomes positive when inside the border and 0 when outside
 
    return (-within_border.x == within_border.y) ?  bgColor : borderColor;
}