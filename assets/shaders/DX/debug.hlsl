cbuffer cbProj : register(b1) {
    float4x4 projection : PROJECTION;
}

cbuffer cbPerObject : register(b2) {
    float3 color : COLOR0;
}

struct VS_INPUT {
	float2 vPos : POSITION0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(projection, float4(Input.vPos, 0.0, 1.0));
    return output;
}

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {  
	return float4(color, 1.0);
}