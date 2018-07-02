#define Blinn_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
	"CBV(b0)," \
	"CBV(b1)," \
	"CBV(b2)," \
	"DescriptorTable(SRV(t0, numDescriptors = 1)), " \
	"StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_LINEAR," \
					   "addressU = TEXTURE_ADDRESS_CLAMP," \
					   "addressV = TEXTURE_ADDRESS_CLAMP," \
					   "addressw = TEXTURE_ADDRESS_CLAMP)"

    

cbuffer viewConstants : register(b0) {
  	float3 eyePos;
    float4x4 view;
    float4x4 proj;  
}

cbuffer cbPerObject : register(b1) {
    float4x4 world : WORLD;
	float4x4 boneOffsets[255];
}

cbuffer material : register ( b2 ) {
	float3 Ka;
	float3 Kd;
	float3 Ks;
	float3 Ke;
	float Ns;
};

Texture2D<float4> tex : register(t0);
SamplerState texSampler : register(s0);

struct VS_INPUT {
	float3 vPos : POSITION0;
	float3 vNorm : NORMAL0;
	float2 vTexCoords : TEXCOORD0;
	uint4 vBoneIds : BLENDINDICES;
	float4 vBoneWeights : BLENDWEIGHTS;
};

struct VS_OUTPUT {
	float3 vNormal : NORMAL0;
	float3 vToCamera : NORMAL1;
	float2 vTex : TEXCOORD0;
    float4 vPosition : SV_POSITION;
};

[RootSignature(Blinn_RootSig)]
VS_OUTPUT VSMain( VS_INPUT Input ) {  
    VS_OUTPUT output;
	
	float4x4 skin = Input.vBoneWeights.x * boneOffsets[Input.vBoneIds.x];
	skin += Input.vBoneWeights.y * boneOffsets[Input.vBoneIds.y];
	skin += Input.vBoneWeights.z * boneOffsets[Input.vBoneIds.z];
	skin += Input.vBoneWeights.w * boneOffsets[Input.vBoneIds.w];
	

	float4 worldPos = mul(world, mul(float4(Input.vPos, 1.f), skin));
	
    output.vPosition = mul(mul(proj, view), worldPos);
	
	output.vToCamera = normalize(eyePos - worldPos.xyz);
	
	output.vNormal = Input.vNorm;//mul(float4(Input.vNorm, 0.f), skin);
    output.vTex = Input.vTexCoords;
    return output;
}

[RootSignature(Blinn_RootSig)]
float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {    
    
	float3 L = normalize(float3(1.f, 1.f, 1.f));
	float3 V = normalize(Input.vToCamera);
	float3 N = normalize(Input.vNormal);
	float3 H = normalize(L + V);
	
	float Ia = 0.05;
	float Id = 1.0; 
	float Is = 1.0;
	
	float3 texColor = Kd * tex.Sample(texSampler, Input.vTex).rgb;
	
	float diffuseTerm = max(dot(L, N), 0.0);
	float specularAngle = saturate(dot(H, N));
	float specularTerm = Ns == 0.f || specularAngle == 0.f ? 0.f : saturate(pow(specularAngle, Ns));

	return float4((Ka * Ia) + (texColor * diffuseTerm * Id) + (Ks * specularTerm * Is), 1.f);
} 
