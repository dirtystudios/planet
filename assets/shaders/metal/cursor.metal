#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float4 position[[attribute(0)]];
};

struct VertexOut {
    float4 position[[position]];
};

vertex VertexOut cursor_vertex(VertexIn vertexAttributes[[stage_in]]) {
    VertexOut outputValue;
    return outputValue;
}

fragment float4 cursor_frag(VertexOut varyingInput[[stage_in]]) { return float4(1, 1, 1, 1); };
