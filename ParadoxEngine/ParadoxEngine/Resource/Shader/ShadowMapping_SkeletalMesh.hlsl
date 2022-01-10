cbuffer StaticMeshConstantBuffer : register(b0)
{
	float4x4 WVP;
};

cbuffer DLightConstantBuffer : register(b1)
{
	float4x4 GBoneTransforms[68];
};

struct VSInput
{
	float3 position    : POSITION;
	float3 normal    : NORMAL;
	float2 uv0        : TEXCOORD0;
	float4 color    : COLOR;
	uint4 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput result;

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = (float)input.BoneWeights.x / 255.f;
	weights[1] = (float)input.BoneWeights.y / 255.f;
	weights[2] = (float)input.BoneWeights.z / 255.f;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(input.position, 1.0f), GBoneTransforms[input.BoneIndices[i]]).xyz;
	}

	input.position = posL;

	result.position = mul(float4(input.position, 1.0f), WVP);
	result.color = float4(1.f, 1.f, 1.f, 1.f);
	return result;
}

void PSMain(PSInput input)
{
}