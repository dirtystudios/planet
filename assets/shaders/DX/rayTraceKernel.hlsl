cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;  
}

RWTexture2D<float4> result : register(u0);
ByteAddressBuffer vertBuff : register(t0);
Texture2D<float4> skyboxtex : register(t1);
SamplerState samplerSky : register(s1);

static const float PI = 3.14159265f;

struct Ray {
    float3 origin;
    float3 direction;
};

Ray CreateRay(float3 origin, float3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

Ray CreateCameraRay(float2 uv) {
    // Transform the camera origin to world space
    float3 origin = mul(view, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    
    // Invert the perspective projection of the view-space position
    float3 direction = mul(proj, float4(uv, 0.0f, 1.0f)).xyz;
    // Transform the direction from camera to world space and normalize
    direction = mul(view, float4(direction, 0.0f)).xyz;
    direction = normalize(direction);
    return CreateRay(origin, direction);
}

[numthreads(8,8,1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    // Get the dimensions of the RenderTexture
    uint width, height;
    result.GetDimensions(width, height);
    // Transform pixel to [-1,1] range
    float2 uv = float2((id.xy + float2(0.5f, 0.5f)) / float2(width, height) * 2.0f - 1.0f);
    // Get a ray for the UVs
    Ray ray = CreateCameraRay(uv);
    
    // write skybox tex

    float theta = acos(ray.direction.y) / -PI;
    float phi = atan2(ray.direction.x, -ray.direction.z) / -PI * 0.5f;
    
    result[id.xy] = float4(skyboxtex.SampleLevel(samplerSky, float2(phi, theta), 0).rgb, 1.f);
}