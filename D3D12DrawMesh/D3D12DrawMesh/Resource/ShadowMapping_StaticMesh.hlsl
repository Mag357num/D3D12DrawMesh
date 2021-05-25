struct LightState
{
	float3 DirectionLightColor;
	float DirectionLightIntensity;
	float3 DirectionLightDir;
};
cbuffer StaticMeshConstantBuffer : register(b0)
{
	float4x4 World;
};

cbuffer LightConstantBuffer : register(b1)
{
	float4x4 VPMatrix;
	float4x4 ScreenMatrix;
	LightState Light;
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
	float4x4 WVP = mul(World, VPMatrix);
	result.position = mul(float4(input.position, 1.0f), WVP);
	result.color = float4(1.f, 1.f, 1.f, 1.f);
	return result;
}

void PSMain(PSInput input)
{
}