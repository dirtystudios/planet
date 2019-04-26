cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;
    float4x4 invProj;
}

cbuffer cbPerObj : register(b2) {
    float2 pixOffset;
    float4 dirLight;
    float seed;
}

struct Sphere {
    float3 position;
    float radius;
    float3 albedo;
    float p0;
    float3 specular;
    float p1;
    float3 emission;
    float smoothness;
};

RWTexture2D<float4> result : register(u0);
Texture2D<float4> skyboxtex : register(t1);
StructuredBuffer<Sphere> spheres : register(t2);
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
    float3 emission;
    float smoothness;
};

RayHit CreateRayHit()
{
    RayHit hit;
    hit.position = float3(0.0f, 0.0f, 0.0f);
    hit.distance = 1.#INF;
    hit.normal = float3(0.0f, 0.0f, 0.0f);
    hit.albedo = float3(0.f, 0.f, 0.f);
    hit.specular = float3(0.f, 0.f, 0.f);
    hit.emission = float3(0.f, 0.f, 0.f);
    hit.smoothness = 0.f;
    return hit;
}

Ray CreateRay(float3 origin, float3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = float3(1.f, 1.f, 1.f);
    return ray;
}

float energy(float3 color)
{
    return dot(color, 1.0f / 3.0f);
}

static float2 _Pixel;
static float2 _Seed;
float rand()
{
    float result = frac(sin(_Seed / 100.0f * dot(_Pixel, float2(12.9898f, 78.233f))) * 43758.5453f);
    _Seed += 1.0f;
    return result;
}

float sdot(float3 x, float3 y, float f = 1.0f)
{
    return saturate(dot(x, y) * f);
}

float SmoothnessToPhongAlpha(float s)
{
    return pow(1000.0f, s * s);
}

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
    // Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = float3(0.0f, 1.0f, 0.0f);
        bestHit.albedo = 0.5f;
        bestHit.specular = 0.33f;
        bestHit.smoothness = 1.0f;
        bestHit.emission = float3(0.f, 0.f, 0.f);
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, Sphere sphere)
{
    // Calculate distance along the ray where the sphere is intersected
    float3 d = ray.origin - sphere.position;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.radius * sphere.radius;
    if (p2sqr < 0)
        return;
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere.position);
        bestHit.albedo = sphere.albedo;
        bestHit.specular = sphere.specular;
        bestHit.emission = sphere.emission;
        bestHit.smoothness = sphere.smoothness;
    }
}

float3x3 GetTangentSpace(float3 normal)
{
    // Choose a helper vector for the cross product
    float3 helper = float3(1, 0, 0);
    if (abs(normal.x) > 0.99f)
        helper = float3(0, 0, 1);
    // Generate vectors
    float3 tangent = normalize(cross(normal, helper));
    float3 binormal = normalize(cross(normal, tangent));
    return float3x3(tangent, binormal, normal);
}

float3 SampleHemisphere(float3 normal, float alpha)
{

    // alpha determines cosine/power of distribution
    // 0 uniform, 1 for cos, higher for phong
    float cosTheta = pow(rand(), 1.f / (alpha + 1.f));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = 2 * PI * rand();
    float3 tangentSpaceDir = float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // Transform direction to world space
    return mul(tangentSpaceDir, GetTangentSpace(normal));
}

RayHit Trace(Ray ray)
{
    RayHit bestHit = CreateRayHit();
    IntersectGroundPlane(ray, bestHit);
    uint numSpheres, stride;
    spheres.GetDimensions(numSpheres, stride);
    for (uint i = 0; i < numSpheres; i++) {
        IntersectSphere(ray, bestHit, spheres[i]);
    }
    return bestHit;
}


float3 Shade(inout Ray ray, RayHit hit)
{
    if (hit.distance < 1.#INF)
    {
        // Calculate chances of diffuse and specular reflection
        hit.albedo = min(1.0f - hit.specular, hit.albedo);
        float specChance = energy(hit.specular);
        float diffChance = energy(hit.albedo);

        // Roulette-select the ray's path
        float roulette = rand();
        if (roulette < specChance)
        {
            // Specular reflection
            ray.origin = hit.position + hit.normal * 0.001f;
            float alpha = SmoothnessToPhongAlpha(hit.smoothness);
            ray.direction = SampleHemisphere(reflect(ray.direction, hit.normal), alpha);
            ray.energy *= (1.0f / specChance) * hit.specular * sdot(hit.normal, ray.direction);
        }
        else if (diffChance > 0 && roulette < specChance + diffChance)
        {
            // Diffuse reflection
            ray.origin = hit.position + hit.normal * 0.001f;
            ray.direction = SampleHemisphere(hit.normal, 1.0f);
            ray.energy *= (1.0f / diffChance) * hit.albedo;
        }
        else
        {
            // Terminate ray
            ray.energy = 0.0f;
        }

        return hit.emission;
    }
    else
    {
        // Erase the ray's energy - the sky doesn't reflect anything
        ray.energy = 0.0f;
        // Sample the skybox and write it
        float theta = acos(ray.direction.y) / PI;
        float phi = atan2(ray.direction.x, -ray.direction.z) / -PI * 0.5f;
        return float3(0.1f,0.1f, 0.1f);//skyboxtex.SampleLevel(samplerSky, float2(phi, theta), 0).xyz;
    }
}

Ray CreateCameraRay(float2 uv) {
    // Transform the camera origin to world space
    //float3 origin = mul(view, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    float3 origin = eyePos;

    // Invert the perspective projection of the view-space position
    float3 direction = mul(invProj, float4(uv, 0.0f, 1.0f)).xyz;
    // Transform the direction from camera to world space and normalize
    direction = mul(view, float4(direction, 0.0f)).xyz;
    direction = normalize(direction);
    return CreateRay(origin, direction);
}

[numthreads(8,8,1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    _Pixel = id.xy;
    _Seed = seed;
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