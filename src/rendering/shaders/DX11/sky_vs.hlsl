cbuffer cbPerObject : register(b0) {
    float4x4 world : WORLD;
}

struct VS_INPUT {
	float3 vPos : POSITION0;
    float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT {
	float3 vTexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    float4 worldPos = mul(world, float4(Input.vPos, 1.f));
    output.vPosition = worldPos.xyww;
    output.vTexCoords = Input.vPos;
    return output;
}