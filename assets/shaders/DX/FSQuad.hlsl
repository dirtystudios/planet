struct VS_OUTPUT {
	float4 position : SV_Position;
	float2 texcoord: TexCoord;
};

VS_OUTPUT VSMain(uint VertexID: SV_VertexID) {
	VS_OUTPUT Out;
	Out.texcoord = float2((VertexID << 1) & 2, VertexID & 2 );
	Out.position = float4(Out.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return Out;
}

Texture2D<float4> tex : register(t10);
SamplerState texSampler : register(s10);

float4 PSMain( VS_OUTPUT Input ) : SV_TARGET {
	return float4(tex.Sample(texSampler, Input.texcoord).rgb, 1.f);
}