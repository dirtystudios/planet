cbuffer cbPerObject : register(b0) {
    float4x4 projection : PROJECTION;
}

struct VS_INPUT {
	float4 vPos : POSITION0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
    output.vPosition = mul(projection, float4(Input.vPos.xy, 0.0, 1.0));
    return output;
}