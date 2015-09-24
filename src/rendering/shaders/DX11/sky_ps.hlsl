TextureCube<float4> cubeTexture : register(t0);
SamplerState samplerCube : register(s0);

struct PS_INPUT {
    float3 TexCoords : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

float3 PSMain( PS_INPUT Input ) : SV_TARGET {  
    return (cubeTexture.Sample(samplerCube, Input.TexCoords).rgb);
}