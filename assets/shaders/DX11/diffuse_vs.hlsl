
cbuffer cbPerObject : register(b0) {
    float4x4 wvp : WORLDVIEWPROJECTION;
}

struct VS_INPUT {
	float3 vPos : POSITION0;
	float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
	float2 vTexCoords : TEXCOORD0;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(wvp, float4(Input.vPos.xy, 0.0, 1.0));
    output.vTexCoords = Input.vTex;
    return output;
}