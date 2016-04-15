// Vertex Shader -- Manual Convert of terrain_vs.glsl to hlsl

cbuffer cbPerObject : register ( b0 ) {
	float4x4 view : VIEWPROJECTION;
	float4x4 proj : PROJECTION;
	float4x4 world : WORLD;
	float4x4 trans;
	float4x4 scale;
	float rradius;
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
	float height = 0.f * terrainHeightTileArray.SampleLevel(heightmapElevationsSampler, float3(Input.vTex, elevations_tile_index), 0);
	float3 normal = terrainNormalTileArray.SampleLevel(heightmapNormalsSampler, float3(Input.vTex, normals_tile_index), 0).xyz;

	float4 pos = float4(Input.vPos.x, Input.vPos.y, rradius / 2.f, 1.f);
	pos = mul(scale, pos);
	pos = mul(trans, pos);
	
	float3 mapped = normalize(pos.xyz) * (height + (rradius / 2.f));
	
	pos = float4(mapped.xyz, 1.f);
	pos = mul(world, pos);
	pos = mul(view, pos);
	pos = mul(proj, pos);
	
	output.vPosition = pos;
	output.vNormal = normal;
    output.vTexture = Input.vTex;
	
	return output;
}    
