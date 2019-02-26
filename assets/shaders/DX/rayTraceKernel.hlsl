cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;  
}

cbuffer cbPerObj : register(b2) {
    float2 pixOffset;
    float4 dirLight;
}

RWTexture2D<float4> result : register(u0);
ByteAddressBuffer vertBuff : register(t0);
Texture2D<float4> skyboxtex : register(t1);
SamplerState samplerSky : register(s1);

static const float PI = 3.14159265f;

struct Ray {
    float3 origin;
    float3 direction;
    float3 energy;
};

struct RayHit
{
    float3 position;
    float distance;
    float3 normal;
    float3 albedo;
    float3 specular;
};

RayHit CreateRayHit()
{
    RayHit hit;
    hit.position = float3(0.0f, 0.0f, 0.0f);
    hit.distance = 1.#INF;
    hit.normal = float3(0.0f, 0.0f, 0.0f);
    hit.albedo = float3(0.f, 0.f, 0.f);
    hit.specular = float3(0.f, 0.f, 0.f);
    return hit;
}

Ray CreateRay(float3 origin, float3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = float3(1.f, 1.f, 1.f);
    return ray;
}

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
    // Calculate distance along the ray where the ground plane is intersected
    float t = ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = float3(0.0f, 1.0f, 0.0f);
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, float4 sphere)
{
    // Calculate distance along the ray where the sphere is intersected
    float3 d = ray.origin - sphere.xyz;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.w * sphere.w;
    if (p2sqr < 0)
        return;
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere.xyz);
    }
}

RayHit Trace(Ray ray)
{
    RayHit bestHit = CreateRayHit();
    IntersectGroundPlane(ray, bestHit);
    int n = 18;
    for (int i = -n; i <= n; ++i)
        for (int j = -n; j <= n; ++j)
            IntersectSphere(ray, bestHit, float4(i * 10.0f, 3.0f, j * 10.0f, 3.0f));
    return bestHit;
}


float3 Shade(inout Ray ray, RayHit hit)
{
    if (hit.distance < 1.#INF)
    {
        float3 specular = float3(0.04f, 0.04f, 0.04f);
        float3 albedo = float3(0.8f, 0.8f, 0.8f);
        // Reflect the ray and multiply energy with specular reflection
        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);
        ray.energy *= specular;
        
        bool shadow = false;
        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, -1 * dirLight.xyz);
        RayHit shadowHit = Trace(shadowRay);
        if (shadowHit.distance != 1.#INF) 
            return float3(0.f, 0.f, 0.f);
        return saturate(dot(hit.normal, dirLight.xyz) * -1) * dirLight.w * albedo;
    }
    else
    {
        // Erase the ray's energy - the sky doesn't reflect anything
        ray.energy = 0.0f;
        // Sample the skybox and write it
        float theta = acos(ray.direction.y) / -PI;
        float phi = atan2(ray.direction.x, -ray.direction.z) / -PI * 0.5f;
        return skyboxtex.SampleLevel(samplerSky, float2(phi, theta), 0).xyz * 1.8f;
    }
}

Ray CreateCameraRay(float2 uv) {
    // Transform the camera origin to world space
    //float3 origin = mul(view, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 origin = eyePos;

    // Invert the perspective projection of the view-space position
    float3 direction = mul(float4(uv, 0.0f, 1.0f), proj).xyz;
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
    float2 uv = float2((id.xy + pixOffset) / float2(width, height) * 2.0f - 1.0f);
    // Get a ray for the UVs
    Ray ray = CreateCameraRay(uv);
    
    // Trace and shade
    float3 res = float3(0, 0, 0);
    for (int i = 0; i < 8; i++)
    {
        RayHit hit = Trace(ray);
        res += ray.energy * Shade(ray, hit);
        if (!any(ray.energy))
            break;
    }

    result[id.xy] = float4(res, 1.f);
}