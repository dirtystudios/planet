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
    float3 toCamera;
};

struct ViewConstants {
    float4x4 proj;
    float4x4 viewCbConstant;
    float3 eyePos;
};

struct ObjConstants {
    float3 textColor;
};

vertex VertexOut text_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(2)]]) {
    VertexOut outputValue;
    outputValue.position = view.proj * view.viewCbConstant * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1);
    outputValue.texture  = attributes.texture;
    outputValue.toCamera = normalize(view.eyePos - float3(outputValue.position.x, outputValue.position.y, outputValue.position.z));
    return outputValue;
}

fragment float4 text_frag(VertexOut varyingInput[[stage_in]], texture2d<float> atlas[[texture(0)]], sampler atlasSampler[[sampler(0)]],
                          constant ObjConstants& obj[[buffer(3)]], constant ViewConstants& view[[buffer(2)]]) {

    
    // Outline of glyph is the isocontour with value 50%
    float edgeDistance = 0.5;
    // Sample the signed-distance field to find distance from this fragment to the glyph outline
    float sampleDistance = atlas.sample(atlasSampler, varyingInput.texture).x;
//    float3 ve = normalize(varyingInput.toCamera) * sampleDistance * .075;
//    sampleDistance = atlas.sample(atlasSampler, varyingInput.texture + float2(ve.x, ve.y)).x;
    // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
    float edgeWidth = 0.75 * length(float2(dfdx(sampleDistance), dfdy(sampleDistance)));
    // Smooth the glyph edge by interpolating across the boundary in a band with the width determined above
    float insideness = smoothstep(edgeDistance - edgeWidth, edgeDistance + edgeWidth, sampleDistance);
    (void)insideness;
    
    insideness = smoothstep(0.25, 0.75, sampleDistance);
    
    
    
    return float4(obj.textColor.x, obj.textColor.y, obj.textColor.z, insideness);
//    return float4(obj.textColor.x, obj.textColor.y, obj.textColor.z, sampleDistance);
};
