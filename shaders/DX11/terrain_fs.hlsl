// Pixel Shader -- Manual Convert of terrain_fs.glsl to hlsl          

struct PS_INPUT {
	float2 vTexture : TEXCOORD0;
	float3 vNormal : NORMAL;
	float4 vPosition : SV_POSITION;
};

float4 PSMain( PS_INPUT Input ) : SV_TARGET {    
    //float3 l = normalize(float3(0, 0, 1));  
    
    float4 vColor = float4(Input.vNormal, 1.f);
    //float d = clamp(dot(l, Input.vCube), 0.f, 1.f);
    //float4 vColor = float4(d, d, d, 1.f);
	return vColor;
} 