cbuffer cbPerObject : register(b0) {
	float4x4 rotation;
}

cbuffer cbConstant : register(b1) {
    float4x4 projection : PROJECTION;
}

struct VS_INPUT {
	float4 vPos : POSITION0;
	//float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT {
	float2 vTexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
	/*float4x4 tmp = mul (projection, rotation);
    output.vPosition = mul(tmp, Input.vPos);
    output.vTexCoords = Input.vTex;*/
	output.vPosition = mul(projection, float4(Input.vPos.xy, 0.0, 1.0));
	output.vTexCoords = Input.vPos.zw;
    return output;
}