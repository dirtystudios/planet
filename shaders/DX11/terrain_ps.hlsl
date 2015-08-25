// Pixel Shader -- Manual Convert of terrain_fs.glsl to hlsl      

// Lets put 60 days on the clock and see how many revisions this takes me         

//uniform sampler2DArray heightmap_normals_tile_array;
//uniform int normals_tile_index;

/* These aren't used currently...
tbuffer tbPerObject {
	sampler g_heightmap_normals_tile_array;
};
cbuffer cbPerObject {
	int g_normals_tile_index;
};*/

struct PS_INPUT {
	float2 vTexture : TEXCOORD0;
	float3 vCube : NORMAL;
	float4 vPosition : SV_POSITION;
};

/*struct PS_OUTPUT {
	float4 vColor : COLOR0;
};*/

//in vec2 t;
//in vec3 c;                                                                     
//out vec4 color;

//void main(void) {
float4 PSMain( PS_INPUT Input ) : SV_TARGET {    
    float3 l = normalize(float3(0, 0, 1));  
	
    //float3 normal = tex2D(g_heightmap_normals_tile_array, float3(Input.vTexture, asfloat(g_normals_tile_index))).xyz;
    
    //color = vec4(normal, 1.f);

    float d = clamp(dot(l, Input.vCube), 0.f, 1.f);
    float4 vColor = float4(d, d, d, 1.f);
	return vColor;
} 