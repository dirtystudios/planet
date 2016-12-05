cbuffer viewConstants : register(b0) {
    float3 eyePos;
    float4x4 view;
    float4x4 proj;
}

cbuffer cbPerObject : register(b1) {
    float4x4 world;
    uint heightmapIndex;
	uint normalmapIndex;
	uint heightmapLod;
    uint lod;
}

Texture2DArray<float> heightmap : register(t0);
SamplerState heightmapSampler : register(s0);

Texture2DArray<float> normalmap : register(t1);
SamplerState normalmapSampler : register (s1);

struct VS_INPUT {
    float3 vPos : POSITION;
    float3 vNorm : NORMAL0;
    float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 vPosition : SV_POSITION;
    float2 vTex : TEXCOORD0;
    float3 vNorm : NORMAL0;
};

float4 getColor(uint index) {
    if (index > 64) {
        return float4(1.f, 1.f, 1.f, 1.f);
    }

    uint colors[64] = {
        0x0000FF, 0xFFFF00, 0x1CE6FF, 0xFF34FF, 0xFF4A46, 0x008941, 0x006FA6, 0xA30059, 0xFFDBE5, 0x7A4900, 0x0000A6, 0x63FFAC, 0xB79762, 0x004D43, 0x8FB0FF, 0x997D87,
        0x5A0007, 0x809693, 0xFEFFE6, 0x1B4400, 0x4FC601, 0x3B5DFF, 0x4A3B53, 0xFF2F80, 0x61615A, 0xBA0900, 0x6B7900, 0x00C2A0, 0xFFAA92, 0xFF90C9, 0xB903AA, 0xD16100,
        0xDDEFFF, 0x000035, 0x7B4F4B, 0xA1C299, 0x300018, 0x0AA6D8, 0x013349, 0x00846F, 0x372101, 0xFFB500, 0xC2FFED, 0xA079BF, 0xCC0744, 0xC0B9B2, 0xC2FF99, 0x001E09,
        0x00489C, 0x6F0062, 0x0CBD66, 0xEEC3FF, 0x456D75, 0xB77B68, 0x7A87A1, 0x788D66, 0x885578, 0xFAD09F, 0xFF8A9A, 0xD157A0, 0xBEC459, 0x456648, 0x0086ED, 0x886F4C,
    };

    uint mask = ~(((1 << 16) - 1) << 8);
    float    r    = (colors[index] >> 16 & mask) / 255.f;
    float    g    = (colors[index] >> 8 & mask) / 255.f;
    float    b    = (colors[index] & mask) / 255.f;
    return float4(r, g, b, 1.f);
}

VS_OUTPUT VSMain(VS_INPUT input) {
	float  height = heightmap.SampleLevel(heightmapSampler, float3(input.vTex, heightmapIndex), 0) * 250.f;
	float4 normal = normalmap.SampleLevel(normalmapSampler, float3(input.vTex, normalmapIndex), 0);
	float4 worldPos = mul(world, float4(input.vPos.x, input.vPos.y, height, 1.f));

    VS_OUTPUT output;
    output.vTex = input.vTex;
    output.vNorm = normal.xyz;
    output.vPosition = mul(mul(proj, view), worldPos);

    return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET {
	float d = clamp(dot(normalize(float3(1, 1, 1)), input.vNorm), 0.1, 1.0);
	return float4(d, d, d, 1.f) * 0.8 + getColor(heightmapLod) * 0.2;
}
