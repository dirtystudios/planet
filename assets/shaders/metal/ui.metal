#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float4 position[[attribute(0)]];
    float2 tex;
};

struct VertexOut {
    float4 position[[position]];
    float2 texture;
};

struct ViewConstants {
    float4x4 proj;
};

struct ObjConstants {
    float4 bgColor;
    float4 borderColor;
    float2 borderSize;
    float3 position;
};

vertex VertexOut ui_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(2)]], constant ObjConstants& obj[[buffer(3)]]) {
    VertexOut outputValue;
    float3 pos = obj.position + attributes.position.xyz;
    outputValue.position = view.proj * float4(pos, 1);
    outputValue.texture  = attributes.tex;
    return outputValue;
}

fragment float4 ui_frag(VertexOut varyingInput[[stage_in]], constant ObjConstants& obj[[buffer(3)]]) {

    float2 within_border = clamp((varyingInput.texture * varyingInput.texture - varyingInput.texture) - (obj.borderSize * obj.borderSize - obj.borderSize), 0.f,
                                 1.f); // becomes positive when inside the border and 0 when outside

    return (-within_border.x == within_border.y) ? obj.bgColor : obj.borderColor;
};
