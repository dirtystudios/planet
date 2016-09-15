cbuffer cbPerObject : register(b2) {
    float3 textColor : COLOR0;
}

cbuffer cbConstant : register(b1) {
    float4x4 projection : PROJECTION;
}

struct VS_INPUT {
	float3 vPos : POSITION0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(projection, float4(Input.vPos.xy, 0.0, 1.0));
    return output;
}

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {  
    float4 vColor = float4(textColor, 1.0);
    return vColor;
}