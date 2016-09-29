#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float3 position[[attribute(0)]];
    float2 texture[[attribute(1)]];
    float3 color[[attribute(2)]];
};

struct VertexOut {
    float4 position[[position]];
    float2 texture;
    float3 color;
};

struct ViewConstants {
    float4x4 view;
    float4x4 proj;
};

vertex VertexOut debug_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(1)]]) {
    VertexOut outputValue;
    outputValue.position = view.proj * view.view * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1);
    outputValue.texture  = attributes.texture;
    outputValue.color    = attributes.color;
    return outputValue;
}

fragment float4 debug_frag(VertexOut varyingInput[[stage_in]]) { return float4(varyingInput.color.x, varyingInput.color.y, varyingInput.color.z, 1.f); };