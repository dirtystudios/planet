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
    float3 normal;
    float3 eye;
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
    
    float4x4 world = view.viewCbConstant;
    float4 worldPosition = world * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1);
    float3 eye = worldPosition.xyz - view.eyePos;
    float3 normal = float3(0, 0, -1);
    
    float3x3 tangentToWorld;
    tangentToWorld[0] = (world * float4( 1,  0,  0, 1)).xyz;
    tangentToWorld[1] = (world * float4( 0,  1,  0, 1)).xyz;
    tangentToWorld[2] = (world * float4( 0,  0, -1, 1)).xyz;
    float3x3 worldToTangent = transpose(tangentToWorld);
    
    outputValue.eye = worldToTangent * eye;
    outputValue.normal = worldToTangent * normal;
    
    outputValue.position = view.proj * worldPosition;
    outputValue.texture  = attributes.texture;
    return outputValue;
}

fragment float4 text_frag(VertexOut varyingInput[[stage_in]], texture2d<float> atlas[[texture(0)]], sampler atlasSampler[[sampler(0)]],
                          constant ObjConstants& obj[[buffer(3)]], constant ViewConstants& view[[buffer(2)]]) {

    
    float4 color = float4(obj.textColor.x, obj.textColor.y, obj.textColor.z, 1.0);
    float sampleDistance = atlas.sample(atlasSampler, varyingInput.texture).x;
//    float3 ve = -1.f * normalize(varyingInput.toCamera) * sampleDistance * .1;
//    sampleDistance = atlas.sample(atlasSampler, varyingInput.texture + float2(ve.x, ve.y)).x;

    bool outline = false;
    bool softEdges = true;
    bool dropShadow = false;
    
    if (softEdges) {
        float edgeDistance = 0.5;
        // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
        float edgeWidth = 0.75 * length(float2(dfdx(sampleDistance), dfdy(sampleDistance)));
        float min0 = edgeDistance - edgeWidth;
        float max0 = edgeDistance + edgeWidth;
        
        color.a *= smoothstep(min0, max0, sampleDistance);
    } else {
        color.a = sampleDistance >= 0.5;
    }
    
    if (outline) {
        float4 outlineColor = float4(0, .5, 0, 1);
        float min0 = 0.25;
        float min1 = .48;
        float max0 = 0.49999;
        float max1 = 0.5;
        float outlineFactor = 1.0;
        
        if (sampleDistance <= max1 && sampleDistance >= min0) {
            if (sampleDistance <= min1) {
                outlineFactor = smoothstep(min0, min1, sampleDistance);
            } else {
                outlineFactor = smoothstep(max1, max0, sampleDistance);
            }
            color = mix(color, outlineColor, outlineFactor);
        }
    }
    
    if (dropShadow) {
        float4 shadowColor = float4(.5, .5, .5, 1);
        float2 offset = varyingInput.toCamera.xy * -0.005;
        float shadowDistance = atlas.sample(atlasSampler, varyingInput.texture + offset).x;
        
        float edgeDistance = 0.45;
        // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
        float edgeWidth = 0.75 * length(float2(dfdx(shadowDistance), dfdy(shadowDistance)));
        float min0 = edgeDistance - edgeWidth;
        float max0 = edgeDistance + edgeWidth;
        
        float shadowAlpha = smoothstep(min0, max0, shadowDistance);
        
        float4 shadow = float4(shadowColor.rgb, shadowColor.a * shadowAlpha);
        color = mix(shadow, color, color.a);
        
    }
    
   
    
    
    
    
    
    
    return float4(varyingInput.eye.x, varyingInput.eye.y, varyingInput.eye.z, 1.0);
};
