cbuffer cbPerObject : register(b0) {
    float3 textColor : COLOR0;
}

struct PS_INPUT {
    float4 vPosition : SV_POSITION;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET {  
    float4 vColor = float4(textColor, 1.0);
    return vColor;
}