cbuffer cbPerVertex : register ( b0 ) {
	float4x4 wvp: WORLDVIEWPROJECTION;
	float4x4 invWV: VIEWPROJECTION;
};

struct VS_OUTPUT {
	float4 vPosition : SV_POSITION;
	float3 vNormal : NORMAL;
	float2 vTex : TEXCOORD0;
};

struct VS_INPUT {
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float2 vTex : TEXCOORD0;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {    
	VS_OUTPUT output;
	output.vNormal = normalize((mul(invWV, float4(Input.vNormal, 1.f)).xyz));
    output.vPosition = mul(wvp, float4(Input.vPosition.xyz, 1.f));
	output.vTex = Input.vTex;
	return output;
}    
