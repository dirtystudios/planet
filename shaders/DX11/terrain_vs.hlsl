// Vertex Shader -- Manual Convert of terrain_vs.glsl to hlsl

/*uniform mat4 view;
uniform mat4 proj;
uniform mat4 world;
uniform int elevations_tile_index;
uniform int normals_tile_index;
uniform sampler2DArray heightmap_elevations_tile_array;
uniform sampler2DArray heightmap_normals_tile_array;*/

cbuffer cbPerObject : register ( b0 ) {
	float4x4 g_view : VIEWPROJECTION;
	float4x4 g_projection : PROJECTION;
	float4x4 g_world : WORLD;
	int g_elevations_tile_index;
	int g_normals_tile_index;
};
SamplerState g_heightmap_elevations_tile_array : register(s0);
SamplerState g_heightmap_normals_tile_array : register(s1);

Texture2DArray terrainHeight : register(t0);
Texture2DArray terrainNormal : register(t1);
 
//layout(location = 0) in vec2 position;                                                  
//layout(location = 1) in vec2 tex; 

struct VS_INPUT {
	float2 vPos : POSITION;
	float2 vTex : TEXCOORD0;
};

//out vec2 t;
//out vec3 c;

struct VS_OUTPUT {
	float2 vTexture : TEXCOORD0;
	float3 vCube : NORMAL;
	float4 vPosition : SV_POSITION;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {    
	VS_OUTPUT output;
    //float height = 50.f * tex2D(g_heightmap_elevations_tile_array, float3(Input.vTex, asfloat(g_elevations_tile_index))).x;
	float height = 50.f * terrainHeight.SampleLevel(g_heightmap_elevations_tile_array, float3(Input.vTex, asfloat(g_elevations_tile_index)), 0).x;
    //float3 normal = tex2D(g_heightmap_normals_tile_array, float3(Input.vTex, asfloat(g_normals_tile_index)));
	float3 normal = terrainNormal.SampleLevel(g_heightmap_normals_tile_array, float3(Input.vTex, asfloat(g_normals_tile_index)), 0).xyz;
    float4 pos = float4(Input.vPos.x, Input.vPos.y, height, 1.f);                  
    output.vCube = normal;
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

    //gl_Position = proj * view * world * pos;
	//output.vPosition = g_projection * g_view * g_world * pos;
	output.vPosition = mul(mul(mul(pos, g_world), g_view), g_projection);
	
	return output;
}    
