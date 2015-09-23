cbuffer cbPerObject : register(b0) {
    float4 bgColor : COLOR0;
    float4 borderColor : COLOR1;
    float2 borderSize;
}

struct PS_INPUT {
    float2 vTexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET {
    float2 within_border = saturate(
                            (Input.vTexCoords * Input.vTexCoords - Input.vTexCoords)
                            - (borderSize * borderSize - borderSize)); //becomes positive when inside the border and 0 when outside
 
    return (-within_border.x == within_border.y) ?  bgColor : borderColor;
}