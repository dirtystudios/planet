// Vertex Shader -- Manual Convert of terrain_vs.glsl to hlsl

cbuffer cbPerObject : register ( b0 ) {
	float4x4 view : VIEWPROJECTION;
	float4x4 proj : PROJECTION;
	float4x4 world : WORLD;
	int elevations_tile_index;
	int normals_tile_index;
};
SamplerState heightmapElevationsSampler : register(s0);
SamplerState heightmapNormalsSampler: register(s1);

Texture2DArray<float> terrainHeightTileArray : register(t0);
Texture2DArray<float4> terrainNormalTileArray : register(t1);

struct VS_INPUT {
	float2 vPos : POSITION;
	float2 vTex : TEXCOORD0;
};

struct VS_OUTPUT {
	float2 vTexture : TEXCOORD0;
	float3 vNormal : NORMAL;
	float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {    
	VS_OUTPUT output;
	float height = 250.f * terrainHeightTileArray.SampleLevel(heightmapElevationsSampler, float3(Input.vTex, elevations_tile_index), 0);
	float3 normal = terrainNormalTileArray.SampleLevel(heightmapNormalsSampler, float3(Input.vTex, normals_tile_index), 0).xyz;
    //float3 normal = {1.0f, 1.0f, 1.0f};
    float4 pos = float4(Input.vPos.x, Input.vPos.y, height, 1.f);                  
    output.vNormal = normal;
    output.vTexture = Input.vTex;

    /*
    // in order to sphereicalize the cube, need to be -1 > x > 1, -1 > y > 1, -1 > z > 1
    float radius = 500;
    vec4 mapping = pos;    

    float x_squared = pos.x * pos.x;
    float y_squared = pos.y * pos.y;
    float z_squared = pos.z * pos.z;
    mapping.x = radius * pos.x * sqrt(1.f - (y_squared / 2.f) - (z_squared / 2.f) + (y_squared * z_squared / 3.f));
    mapping.y = radius * pos.y * sqrt(1.f - (z_squared / 2.f) - (x_squared / 2.f) + (z_squared * x_squared / 3.f));
    mapping.z = (radius + height) * pos.z * sqrt(1.f - (x_squared / 2.f) - (y_squared / 2.f) + (x_squared * y_squared / 3.f));
    */  
	
	pos = mul(world, pos);
	pos = mul(view, pos);
	pos = mul(proj, pos);
	
	output.vPosition = pos;
	
	return output;
}    