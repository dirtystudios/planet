#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float3 position[[attribute(0)]];
};

struct VertexOut {
    float4 position[[position]];
    float3 texture;
};

struct ViewConstants {
    float3   eyePos;
    float4x4 view;
    float4x4 proj;
};

struct ObjectConstants {
    float4x4 world;
};

vertex VertexOut sky_vertex(VertexIn vertexAttributes[[stage_in]], constant ViewConstants& view[[buffer(1)]], constant ObjectConstants& obj[[buffer(2)]]) {
    float4 pos = float4(vertexAttributes.position.x, vertexAttributes.position.y, vertexAttributes.position.z, 1);
    pos        = view.proj * view.view * obj.world * pos;

    VertexOut outputValue;
    outputValue.position = float4(pos.x, pos.y, pos.w, pos.w);
    outputValue.texture  = vertexAttributes.position;
    return outputValue;
}

fragment float4 sky_frag(VertexOut varyingInput[[stage_in]], texturecube<float> skybox[[texture(0)]], sampler skyboxSampler[[sampler(0)]]) {

    return skybox.sample(skyboxSampler, varyingInput.texture);
    // return float4(1, 1, 1, 1);
};
