cbuffer cbPerObject : register(b0) {
	float4x4 rotation;
}

cbuffer cbConstant : register(b1) {
    float4x4 projection : PROJECTION;
}

cbuffer cbPerObject : register(b2) {
    float4 bgColor : COLOR0;
    float4 borderColor : COLOR1;
    float2 borderSize;
}

struct VS_INPUT {
	float4 vPos : POSITION0;
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

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {
    float2 within_border = saturate(
                            (Input.vTexCoords * Input.vTexCoords - Input.vTexCoords)
                            - (borderSize * borderSize - borderSize)); //becomes positive when inside the border and 0 when outside
 
    return (-within_border.x == within_border.y) ?  bgColor : borderColor;
}