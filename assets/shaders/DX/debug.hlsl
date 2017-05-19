cbuffer cbProj : register(b0) {
    float4x4 view : VIEW;
    float4x4 projection : PROJECTION;
}

Texture2D<float> text : register(t0);
SamplerState textSampler : register(s0);

struct VS_INPUT {
	float3 vPos : POSITION0;
    float2 vTex : TEXCOORD0;
    float4 vCol : COLOR0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
    float2 vTex : TEXCOORD0;
    float4 vCol : COLOR0;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(mul(projection, view), float4(Input.vPos, 1.0));
    output.vTex = Input.vTex;
    output.vCol = Input.vCol;
    return output;
}

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {  
	float alpha = text.Sample(textSampler, Input.vTex);
	return float4(Input.vCol.rgb, alpha * Input.vCol.a);
}