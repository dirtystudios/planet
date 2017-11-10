#include <metal_stdlib>
#include <metal_types.h>

using namespace metal;

struct VertexIn {
    float3 position[[attribute(0)]];
    float3 normal[[attribute(1)]];
    float2 texture[[attribute(2)]];
	uint4 boneIds[[attribute(3)]];
	float4 boneWeights[[attribute(4)]];
};

struct VertexOut {
    float4 position[[position]];
    float3 normal;
    float3 toCamera;
    float2 texture;
};

struct ViewConstants {
    float3   eyePos;
    float4x4 view;
    float4x4 proj;
};

struct ObjectConstants {
    float4x4 world;
	float4x4 boneOffsets[255];
};

struct MaterialConstants {
    float3 ka;
    float3 kd;
    float3 ks;    
    float3 ke;
    float  ns;
};

vertex VertexOut blinn_vertex(VertexIn attributes[[stage_in]], constant ViewConstants& view[[buffer(1)]], constant ObjectConstants& obj[[buffer(2)]]) {
    float4x4 BoneTransform = obj.offsets[a_boneIds.x] * attributes.boneWeights.x;
    BoneTransform     += obj.offsets[a_boneIds.y] * attributes.boneWeights.y;
    BoneTransform     += obj.offsets[a_boneIds.z] * attributes.boneWeights.z;
    BoneTransform     += obj.offsets[a_boneIds.w] * attributes.boneWeights.w;
	
    float4 worldPos = obj.world * (float4(attributes.position.x, attributes.position.y, attributes.position.z, 1.f) * BoneTransform);

    VertexOut outputValue;
    outputValue.texture  = attributes.texture;
    outputValue.normal   = attributes.normal;
    outputValue.toCamera = normalize(view.eyePos - float3(worldPos.x, worldPos.y, worldPos.z));
    outputValue.position = view.proj * view.view * worldPos;
    return outputValue;
}

fragment float4 blinn_frag(VertexOut varyingInput[[stage_in]], texture2d<float> diffuse[[texture(0)]], sampler diffuseSampler[[sampler(0)]],
                           constant MaterialConstants& material[[buffer(3)]]) {
    float3 L = normalize(float3(1, 1, 1));
    float3 V = normalize(varyingInput.toCamera);
    float3 N = normalize(varyingInput.normal);
    float3 H = normalize(L + V);

    // fake light parameters
    float Ia = 0.05f;
    float Id = 1.f;
    float Is = 1.f;

    float4 sampledKd = diffuse.sample(diffuseSampler, varyingInput.texture);
    float3 Kd        = material.kd * float3(sampledKd.x, sampledKd.y, sampledKd.z);
    float3 Ks        = material.ks;
    float3 Ka        = material.ka;

    float diffuseTerm   = max(dot(L, N), 0.0);
    float specularAngle = clamp(dot(H, N), 0.f, 1.f);
    float specularTerm  = material.ns == 0.f || specularAngle == 0.f ? 0.f : clamp(pow(specularAngle, material.ns), 0.f, 1.f);

    return float4((Ka * Ia) + (Kd * diffuseTerm * Id) + (Ks * specularTerm * Is), 1.0f);
};
