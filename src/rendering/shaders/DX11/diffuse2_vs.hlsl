cbuffer cbPerVertex : register ( b0 ) {
	float4x4 wvp: WORLDVIEWPROJECTION;
	float4x4 invWV: VIEWPROJECTION;
};

struct VS_OUTPUT {
	float4 vPosition : SV_POSITION;
	float3 vNormal : NORMAL;
};

struct VS_INPUT {
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
};

VS_OUTPUT VSMain( VS_INPUT Input ) {    
	VS_OUTPUT output;
	output.vNormal = normalize((mul(invWV, float4(Input.vNormal, 1.f)).xyz));
    output.vPosition = mul(wvp, float4(Input.vPosition.xyz, 1.f));
	return output;
}    
