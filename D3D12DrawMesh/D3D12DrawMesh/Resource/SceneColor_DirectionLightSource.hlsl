cbuffer StaticMeshConstantBuffer : register(b0)
{
	float4x4 World;
};

cbuffer CameraConstantBuffer : register(b1)
{
	float4x4 CameraVP;
	float3 CamEye;
};

struct VSInput
{
	float3 position    : POSITION;
	float3 normal    : NORMAL;
	float2 uv0        : TEXCOORD0;
	float4 color    : COLOR;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput result;
	result.position = mul(float4(input.position, 1.0f), World);
	result.position = mul(result.position, CameraVP);
	result.color = float4(1.f, 1.f, 1.f, 1.f);
	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return input.color;
}