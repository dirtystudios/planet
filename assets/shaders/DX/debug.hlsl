cbuffer cbProj : register(b0) {
    float4x4 view : VIEW;
    float4x4 projection : PROJECTION;
}

struct VS_INPUT {
	float3 vPos : POSITION0;
    float2 vTex : TEXCOORD0;
    float3 vCol : COLOR0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
    float2 vTex : TEXCOORD0;
    float3 vCol : COLOR0;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(mul(projection, view), float4(Input.vPos, 1.0));
    output.vTex = Input.vTex;
    output.vCol = Input.vCol;
    return output;
}

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {  
	return float4(Input.vCol, 1.0);
}