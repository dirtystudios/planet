#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float3 position[[attribute(0)]];
    float2 uv[[attribute(1)]];
};

struct VertexOut {
    float4 position[[position]];
    float2 uv;
    float3 tangentEyePos;
    float3 tangentLightPos;
    float3 tangentFragPos;
};

struct ViewConstants {
    float4x4 proj;
    float4x4 view;
    float3 eyePos;
};

struct ObjConstants {
    float4x4 world;
    float4x4 norm;
    float3 textColor;
};

vertex VertexOut text_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(2)]], constant ObjConstants& obj[[buffer(3)]]) {
    VertexOut outputValue;
    
    float3 lightPos = float3(100, 0, 100);
    
    float4 worldPos = obj.world * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1.f);
    float3x3 normalMatrix = float3x3(obj.norm[0].xyz, obj.norm[1].xyz, obj.norm[2].xyz);
    float3 T = normalize(normalMatrix * float3(1, 0, 0));
    float3 N = normalize(normalMatrix * float3(0, 0, 1));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = transpose(float3x3(T, B, N));
    outputValue.tangentEyePos = TBN * view.eyePos.xyz;
    outputValue.tangentLightPos = TBN * lightPos;
    outputValue.tangentFragPos  = TBN * worldPos.xyz;
    outputValue.position = view.proj * view.view * worldPos;
    outputValue.uv = attributes.uv;
    
    return outputValue;
}

float sampleDepth(float2 uv, texture2d<float> depthTexture, sampler depthSampler)
{
    float height =  depthTexture.sample(depthSampler, uv).x;
    return 1.0 - step(0.5, height);
//    return 1 - smoothstep(0.5, 1, height);
}

float2 ParallaxMapping(float2 uv, float3 viewDir, texture2d<float> depthTexture, sampler depthSampler)
{
    
    float heightScale = 0.05;
    // number of depth layers
    const float minLayers = 16;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    float2 P = viewDir.xy / viewDir.z * heightScale;
    float2 deltaTexCoords = P / numLayers;
    
    // get initial values
    float2  currentTexCoords     = uv;
    float currentDepthMapValue = sampleDepth(currentTexCoords, depthTexture, depthSampler);
    
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = sampleDepth(currentTexCoords, depthTexture, depthSampler);
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    
    // get texture coordinates before collision (reverse operations)
    float2 prevTexCoords = currentTexCoords + deltaTexCoords;
    
    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    
    ;
    float beforeDepth = sampleDepth(prevTexCoords, depthTexture, depthSampler) - currentLayerDepth + layerDepth;
    
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    float2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    
    return finalTexCoords;
}

fragment float4 text_frag(VertexOut varyingInput[[stage_in]],
                          texture2d<float> depthTexture[[texture(0)]], sampler depthSampler[[sampler(0)]],
                          texture2d<float> normalTexture[[texture(1)]], sampler normalSampler[[sampler(1)]],
                          constant ViewConstants& view[[buffer(2)]],
                          constant ObjConstants& obj[[buffer(3)]]) {
    
    
    
    // offset texture coordinates with Parallax Mapping
    float3 viewDir   = normalize(varyingInput.tangentEyePos - varyingInput.tangentFragPos);
    float2 texCoords = ParallaxMapping(varyingInput.uv, viewDir, depthTexture, depthSampler);
    
    float depth = depthTexture.sample(depthSampler, texCoords).x;
    float3 color = depth >= 0.5 ? float3(1) : float3(0);
    float3 normal = normalTexture.sample(normalSampler, varyingInput.uv).xyz;
    
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    
    // ambient
    float3 ambient = 0.1 * color;
    // diffuse
    float3 lightDir = normalize(varyingInput.tangentLightPos - varyingInput.tangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * color;
    // specular
//    float3 reflectDir = reflect(-lightDir, normal);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    float3 specular = float3(0.2) * spec;
    
//    return float4(normal, 1.0);
    return float4(ambient + diffuse + specular, 1.0);
    
    
    
}


/*
fragment float4 text_frag(VertexOut varyingInput[[stage_in]],
                          texture2d<float> atlas[[texture(0)]], sampler atlasSampler[[sampler(0)]],
                          texture2d<float> normalTexture[[texture(1)]], sampler normalSampler[[sampler(1)]],
                          constant ViewConstants& view[[buffer(2)]],
                          constant ObjConstants& obj[[buffer(3)]]) {

    
    float4 color = float4(obj.textColor.x, obj.textColor.y, obj.textColor.z, 1.0);
    float sampleDistance = normalTexture.sample(normalSampler, varyingInput.uv).x;
//    float3 ve = -1.f * normalize(varyingInput.toCamera) * sampleDistance * .1;
//    sampleDistance = atlas.sample(atlasSampler, varyingInput.texture + float2(ve.x, ve.y)).x;

//    bool outline = false;
    bool softEdges = true;
//    bool dropShadow = false;
    
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
    
//    if (outline) {
//        float4 outlineColor = float4(0, .5, 0, 1);
//        float min0 = 0.25;
//        float min1 = .48;
//        float max0 = 0.49999;
//        float max1 = 0.5;
//        float outlineFactor = 1.0;
//
//        if (sampleDistance <= max1 && sampleDistance >= min0) {
//            if (sampleDistance <= min1) {
//                outlineFactor = smoothstep(min0, min1, sampleDistance);
//            } else {
//                outlineFactor = smoothstep(max1, max0, sampleDistance);
//            }
//            color = mix(color, outlineColor, outlineFactor);
//        }
//    }
//
//    if (dropShadow) {
//        float4 shadowColor = float4(.5, .5, .5, 1);
//        float2 offset = varyingInput.toCamera.xy * -0.005;
//        float shadowDistance = atlas.sample(atlasSampler, varyingInput.texture + offset).x;
//
//        float edgeDistance = 0.45;
//        // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
//        float edgeWidth = 0.75 * length(float2(dfdx(shadowDistance), dfdy(shadowDistance)));
//        float min0 = edgeDistance - edgeWidth;
//        float max0 = edgeDistance + edgeWidth;
//
//        float shadowAlpha = smoothstep(min0, max0, shadowDistance);
//
//        float4 shadow = float4(shadowColor.rgb, shadowColor.a * shadowAlpha);
//        color = mix(shadow, color, color.a);
//
//    }
    
   
    
    
    
    
    
    
//    return float4(sampleDistance, sampleDistance, sampleDistance, 1.0);
//    return color;
    return float4(normalTexture.sample(normalSampler, varyingInput.uv).xyz, 1.0);
};

*/
