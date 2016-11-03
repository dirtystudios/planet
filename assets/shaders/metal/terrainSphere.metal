#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float3 position[[attribute(0)]];
    float3 normal[[attribute(1)]];
    float2 texture[[attribute(2)]];
};

struct VertexOut {
    float4 position[[position]];
    float2 texture[[attribute(2)]];
    float3 normal;
};

struct ViewConstants {
    float3   eyePos;
    float4x4 view;
    float4x4 proj;
};

struct TileConstants {
    float4x4 world;
    uint     heightmapIndex;
};

vertex VertexOut terrainSphere_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(1)]], constant TileConstants& tile[[buffer(2)]],
                                      texture2d_array<float> heightmap[[texture(0)]], sampler heightmapSampler[[sampler(0)]]) {
    float  height   = heightmap.sample(heightmapSampler, attributes.texture, tile.heightmapIndex).x * 75.f;
    float4 worldPos = tile.world * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1.f);
    float3 p        = normalize(worldPos.xyz) * (height + 256.f / 2.f);
    worldPos        = float4(p.x, p.y, p.z, 1.f);

    VertexOut outputValue;
    outputValue.texture = attributes.texture;
    outputValue.normal  = attributes.normal;

    outputValue.position = view.proj * view.view * worldPos;
    return outputValue;
}

fragment float4 terrainSphere_frag(VertexOut varyingInput[[stage_in]], constant TileConstants& tile[[buffer(2)]], texture2d_array<float> heightmap[[texture(0)]],
                                   sampler heightmapSampler[[sampler(0)]]) {

    //    return float4(heightmap.sample(heightmapSampler, varyingInput.texture, tile.heightmapIndex));
    return float4(varyingInput.texture.x, varyingInput.texture.y, 0, 1);
};
