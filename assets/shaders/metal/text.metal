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
    float4 textColor;
    float4 lightPos;
    float heightScale;
};

vertex VertexOut text_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(2)]], constant ObjConstants& obj[[buffer(3)]]) {
    VertexOut outputValue;
    
    float3 lightPos = obj.lightPos.xyz;
    
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
    return 1.0 - step(0.49, height);
}

float2 ParallaxMapping(float heightScale, float2 uv, float3 viewDir, texture2d<float> depthTexture, sampler depthSampler)
{
    
    
    
    bool doReliefMapping = true;
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 8;
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
    
    if (doReliefMapping) {
        // decrease shift and height of layer by half
        float2 deltaTexCoord = deltaTexCoords / 2;
        float deltaHeight = layerDepth / 2;
        
        // return to the mid point of previous layer
        currentTexCoords += deltaTexCoord;
        currentLayerDepth -= deltaHeight;
        
        // binary search to increase precision of Steep Paralax Mapping
        const int numSearches = 5;
        for(int i=0; i<numSearches; i++)
        {
            // decrease shift and height of layer by half
            deltaTexCoord /= 2;
            deltaHeight /= 2;
            
            // new depth from heightmap
            currentDepthMapValue = sampleDepth(currentTexCoords, depthTexture, depthSampler);
            
            // shift along or agains vector V
            if(currentDepthMapValue > currentLayerDepth) // below the surface
            {
                currentTexCoords -= deltaTexCoord;
                currentLayerDepth += deltaHeight;
            }
            else // above the surface
            {
                currentTexCoords += deltaTexCoord;
                currentLayerDepth -= deltaHeight;
            }
        }
        
        return currentTexCoords;
    } else {
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
}

//fragment float4 text_frag(VertexOut varyingInput[[stage_in]],
//                          texture2d<float> depthTexture[[texture(0)]], sampler depthSampler[[sampler(0)]],
//                          texture2d<float> normalTexture[[texture(1)]], sampler normalSampler[[sampler(1)]],
//                          constant ViewConstants& view[[buffer(2)]],
//                          constant ObjConstants& obj[[buffer(3)]]) {
//
//
//
//    // offset texture coordinates with Parallax Mapping
//    float3 viewDir   = normalize(varyingInput.tangentEyePos - varyingInput.tangentFragPos);
//    float2 texCoords = ParallaxMapping(obj.heightScale, varyingInput.uv, viewDir, depthTexture, depthSampler);
//
//    float depth = 1 - sampleDepth(texCoords, depthTexture, depthSampler);
////    float3 color =     depth >= 0.51 ? float3(1) : depth >= 0.49f ? float3(0, 0.75, 0) : float3(0);
////    float3 color = depth > 0.5 ? float3(1) : float3(0, 0, 0);
//    float3 color = float3(depth);
//    float3 normal = normalTexture.sample(normalSampler, texCoords).xyz;
//
//
//    // transform normal vector to range [-1,1]
//    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
//
//    // ambient
//    float3 ambient = 0.5 * color;
//    // diffuse
//    float3 lightDir = normalize(varyingInput.tangentLightPos - varyingInput.tangentFragPos);
//    float diff = max(dot(lightDir, normal), 0.0);
//    float3 diffuse = diff * color;
//    // specular
////    float3 reflectDir = reflect(-lightDir, normal);
//    float3 halfwayDir = normalize(lightDir + viewDir);
//    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
//
//    float3 specular = float3(0.2) * spec;
//
////    return float4(normal, 1.0);
//
//        float edgeDistance = 0.5;
//        // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
//        float edgeWidth = 0.75 * length(float2(dfdx(depth), dfdy(depth)));
//        float min0 = edgeDistance - edgeWidth;
//        float max0 = edgeDistance + edgeWidth;
//
//        float alpha = smoothstep(min0, max0, depth);
//    alpha = 1;
//    return float4(ambient + diffuse + specular, alpha);
//
//
//
//}

fragment float4 text_frag(VertexOut varyingInput[[stage_in]],
                          texture2d<float> atlas[[texture(0)]], sampler atlasSampler[[sampler(0)]],
                          texture2d<float> normalTexture[[texture(1)]], sampler normalSampler[[sampler(1)]],
                          constant ViewConstants& view[[buffer(2)]],
                          constant ObjConstants& obj[[buffer(3)]]) {

    float3 viewDir   = normalize(varyingInput.tangentEyePos - varyingInput.tangentFragPos);
//    float2 texCoords = ParallaxMapping(obj.heightScale, varyingInput.uv, viewDir, atlas, atlasSampler);
    float2 texCoords = varyingInput.uv;
    
    
//    float sampleDistance = atlas.sample(atlasSampler, texCoords).x;
    float sampleDistance = atlas.sample(atlasSampler, texCoords, gradient2d(viewDir.xy / viewDir.z, viewDir.xy / viewDir.z)).x;
    float3 normal = sampleDistance >= 0.5 ? float3(0, 0, 1) : normalTexture.sample(normalSampler, texCoords).xyz;
    
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    
    float3 color = obj.textColor.xyz;
    float alpha = 1.0;
    
    bool outline = false;
    bool softEdges = true;
//    bool dropShadow = false;
    
    if (softEdges) {
        float edgeDistance = 0.48;
        // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
        float edgeWidth = 0.75 * length(float2(dfdx(sampleDistance), dfdy(sampleDistance)));
        float min0 = edgeDistance - edgeWidth;
        float max0 = edgeDistance + edgeWidth;
        
        alpha *= smoothstep(min0, max0, sampleDistance);
    } else {
        alpha = sampleDistance >= 0.5;
    }
    
    if (outline) {
        float3 outlineColor = float3(0, .5, 0);
        float min0 = 0.25;
        float min1 = .47;
        float max0 = 0.4799;
        float max1 = 0.48;
        float outlineFactor = 1.0;

        if (sampleDistance <= max1 && sampleDistance >= min0) {
            if (sampleDistance <= min1) {
                outlineFactor = smoothstep(min0, min1, sampleDistance);
            } else {
                outlineFactor = smoothstep(max1, max0, sampleDistance);
            }
            color = mix(color, outlineColor, outlineFactor);
            alpha = mix(alpha, 1.0, outlineFactor);
        }
    }
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
    
   
    
    
    // crappy lighting
    
    // ambient
    float3 ambient = 0.5 * color;
    // diffuse
    float3 lightDir = normalize(varyingInput.tangentLightPos - varyingInput.tangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    float3 diffuse = diff * color;
    // specular
    //    float3 reflectDir = reflect(-lightDir, normal);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    float3 specular = float3(0.2) * spec;
    
    return float4(ambient + diffuse + specular, alpha);
    
    
//    return float4(normal, 1.0);
//    return color;
};


