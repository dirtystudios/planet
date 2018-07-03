#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float3 position[[attribute(0)]];
    float2 texture[[attribute(1)]];
};

struct VertexOut {
    float4 position[[position]];
    float2 texture;
};

struct ViewConstants {
    float4x4 proj;
    float4x4 viewCbConstant;
    float3 eyeDir;
};

struct ObjConstants {
    float3 textColor;
};

vertex VertexOut text_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(2)]]) {
    VertexOut outputValue;
    outputValue.position = view.proj * view.viewCbConstant * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1);
    outputValue.texture  = attributes.texture;
    return outputValue;
}

fragment float4 text_frag(VertexOut varyingInput[[stage_in]], texture2d<float> atlas[[texture(0)]], sampler atlasSampler[[sampler(0)]],
                          constant ObjConstants& obj[[buffer(3)]], constant ViewConstants& view[[buffer(2)]]) {

    float alpha = atlas.sample(atlasSampler, varyingInput.texture).x;
    return float4(obj.textColor.x, obj.textColor.y, obj.textColor.z, alpha);
};
