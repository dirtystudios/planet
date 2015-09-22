cbuffer cbPerObject : register(b0) {
    float3 textColor : COLOR0;
}

Texture2D<float> text : register(t0);
SamplerState textSampler : register(s0);

struct PS_INPUT {
    float2 TexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET {  

    float4 sampled = float4(1.0, 1.0, 1.0, text.Sample(textSampler, Input.TexCoords).r);
    float4 vColor = float4(textColor, 1.0) * sampled;
    return vColor;
}