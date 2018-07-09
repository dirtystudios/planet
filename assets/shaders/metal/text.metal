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
    float4x4 view;
    float4x4 normalMatrix;
    float3 eyePos;
};

struct ObjConstants {
    float3 textColor;
};

vertex VertexOut text_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(2)]]) {
    VertexOut outputValue;
    
    float4x4 world = view.view;
    
    float4 worldPosition = world * float4(attributes.position.x, attributes.position.y, attributes.position.z, 1);
    float3 worldNormal = (view.normalMatrix * float4(0, 0, 1, 1)).xyz;
    float3 worldTangent = (view.normalMatrix * float4(1, 0, 0, 1)).xyz;
    float3 worldBitangent = cross(worldNormal, worldTangent) * 1.f; // multiply for handidness
    
    // calculate vectors to the camera and to the light
    //float3 worldDirectionToLight = normalize(u_light_position - worldPosition.xyz);
    float3 worldDirectionToCamera = normalize(view.eyePos - worldPosition.xyz);
    
    
    
    outputValue.eye = float3(dot(worldDirectionToCamera, worldTangent),
                             dot(worldDirectionToCamera, worldBitangent),
                             dot(worldDirectionToCamera, worldNormal));
    outputValue.position = view.proj * worldPosition;
    outputValue.texture  = attributes.texture;
    
//    float3x3 tangentToWorld;
//    tangentToWorld[0] = (world * float4( 1,  0,  0, 1)).xyz;
//    tangentToWorld[1] = (world * float4( 0,  1,  0, 1)).xyz;
//    tangentToWorld[2] = (world * float4( 0,  0,  1, 1)).xyz;
//    float3x3 worldToTangent = transpose(tangentToWorld);
    
//    outputValue.eye = normalize(worldToTangent * eye);
//    outputValue.normal = normalize(worldNormal);
//    outputValue.position = view.proj * worldPosition;
//    outputValue.texture  = attributes.texture;
///    outputValue.toCamera = normalize(eye);
    return outputValue;
}

fragment float4 text_frag(VertexOut varyingInput[[stage_in]], texture2d<float> atlas[[texture(0)]], sampler atlasSampler[[sampler(0)]],
                          constant ObjConstants& obj[[buffer(3)]], constant ViewConstants& view[[buffer(2)]]) {

    float2 T = varyingInput.texture;
    float parallaxScale = -0.0015f;
    float3 V = normalize(varyingInput.eye);
    
    
    // determine number of layers from angle between V and N
    const float minLayers = 5;
    const float maxLayers = 100;
    float numLayers = mix(maxLayers, minLayers, abs(dot(float3(0, 0, 1), V)));
    
    // height of each layer
    float layerHeight = 1.0 / numLayers;
    // depth of current layer
    float currentLayerHeight = 0;
    // shift of texture coordinates for each iteration
    float2 dtex = parallaxScale * V.xy / V.z / numLayers;
    
    // current texture coordinates
    float2 currentTextureCoords = T;
    
    // get first depth from heightmap
    float heightFromTexture =  atlas.sample(atlasSampler, currentTextureCoords).r;
    heightFromTexture = step(0.5f, heightFromTexture);
    
    // while point is above surface
    while(heightFromTexture > currentLayerHeight)
    {
        // to the next layer
        currentLayerHeight += layerHeight;
        // shift texture coordinates along vector V
        currentTextureCoords -= dtex;
        // get new depth from heightmap
        heightFromTexture =  atlas.sample(atlasSampler, currentTextureCoords).r;
        heightFromTexture = step(0.5f, heightFromTexture);
    }
    
    ///////////////////////////////////////////////////////////
    // Start of Relief Parallax Mapping
    
    // decrease shift and height of layer by half
    float2 deltaTexCoord = dtex / 2;
    float deltaHeight = layerHeight / 2;
    
    // return to the mid point of previous layer
    currentTextureCoords += deltaTexCoord;
    currentLayerHeight -= deltaHeight;
    
    // binary search to increase precision of Steep Paralax Mapping
    const int numSearches = 5;
    for(int i=0; i<numSearches; i++)
    {
        // decrease shift and height of layer by half
        deltaTexCoord /= 2;
        deltaHeight /= 2;
        
        // new depth from heightmap
        heightFromTexture = atlas.sample(atlasSampler, currentTextureCoords).r;
        heightFromTexture = step(0.5f, heightFromTexture);
        
        // shift along or agains vector V
        if(heightFromTexture > currentLayerHeight) // below the surface
        {
            currentTextureCoords -= deltaTexCoord;
            currentLayerHeight += deltaHeight;
        }
        else // above the surface
        {
            currentTextureCoords += deltaTexCoord;
            currentLayerHeight -= deltaHeight;
        }
    }
    
    // return results
    T = currentTextureCoords;
    
//------------
    
    
    
    
    float sampleDistance = atlas.sample(atlasSampler, T).r;
    sampleDistance = step(0.5f, sampleDistance);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    float4 color = float4(obj.textColor.x, obj.textColor.y, obj.textColor.z, 1.0);
//    float sampleDistance = atlas.sample(atlasSampler, varyingInput.texture).x;
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
    
   
    
    
    
    
    
    
//    return float4(sampleDistance, sampleDistance, sampleDistance, 1.0);
    return color;
};
