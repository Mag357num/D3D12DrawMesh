Texture2D shadowMap : register(t0);
SamplerState sampleClamp : register(s0);

struct DirectionalLightState
{
	float3 Color;
	float3 Dir;
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
};

cbuffer StaticMeshConstantBuffer : register(b0)
{
	float4x4 World;
};

cbuffer CameraConstantBuffer : register(b1)
{
	float4x4 CameraVP;
	float3 CamEye;
};

cbuffer DLightConstantBuffer : register(b2)
{
	float4x4 VPMatrix;
	float4x4 ScreenMatrix;
	DirectionalLightState DirectionalLight;
};

float CalcUnshadowedAmountPCF2x2(float4 ScreenSpacePos, float bias)
{
	float2 TexCoord = ScreenSpacePos.xy;
	float ActualDepth = ScreenSpacePos.z - bias;

	uint width, height, numMips;
	shadowMap.GetDimensions(0, width, height, numMips);
	float2 ShadowMapDims = float2(width, height);

	float4 PixelCoordFrac = float4(1.0f, 1.0f, 1.0f, 1.0f);
	PixelCoordFrac.xy = frac(ShadowMapDims * TexCoord);
	PixelCoordFrac.zw = 1.0f - PixelCoordFrac.xy;
	float4 BilinearWeights = PixelCoordFrac.zxzx * PixelCoordFrac.wwyy;

	float2 TexelUnits = 1.0f / ShadowMapDims;
	float4 SampleDepths;
	SampleDepths.x = shadowMap.Sample(sampleClamp, TexCoord);
	SampleDepths.y = shadowMap.Sample(sampleClamp, TexCoord + float2(TexelUnits.x, 0.0f));
	SampleDepths.z = shadowMap.Sample(sampleClamp, TexCoord + float2(0.0f, TexelUnits.y));
	SampleDepths.w = shadowMap.Sample(sampleClamp, TexCoord + TexelUnits);

	float4 ShadowTests = (SampleDepths > ActualDepth) ? 1.0f : 0.0f;
	return dot(BilinearWeights, ShadowTests);
}

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
	float4 worldpos : POSITION;
	float4 shadowScreenPos :POSITION1;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput result;
	result.position = mul(float4(input.position, 1.0f), World);
	result.worldpos = result.position;
	result.position = mul(result.worldpos, CameraVP);
	result.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
	result.color = float4(1.f, 1.f, 1.f, 1.f);
	result.shadowScreenPos = mul(result.worldpos, mul(VPMatrix, ScreenMatrix));

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	input.normal = input.normal;
	float3 viewDir = CamEye.xyz - input.worldpos.xyz;
	viewDir = normalize(viewDir);
	float3 dir = normalize(DirectionalLight.Dir * -1.f);
	float3 halfWay = normalize(viewDir + dir);

	float shine = 10.f;
	float4 specularColor;
	specularColor = float4(DirectionalLight.Specular, 1.5f) * float4(DirectionalLight.Color, 1.f) * pow(max(dot(input.normal, halfWay), 0.f), shine);
	specularColor *= dot(input.normal, dir);

	float4 difuseColor = float4(DirectionalLight.Diffuse, 0.3f) * float4(DirectionalLight.Color, 1.f) * max(dot(input.normal, DirectionalLight.Dir.xyz * -1.f), 0.f);

	float4 ambientColor = float4(DirectionalLight.Ambient, 0.02f) * float4(DirectionalLight.Color, 1.f);

	float bias = max(0.005f * (1.0f - abs(dot(input.normal, DirectionalLight.Dir))), 0.00005f);
	float ShadowFactor = CalcUnshadowedAmountPCF2x2(input.shadowScreenPos, bias);
	float4 FrameBuffer = (ambientColor + 0.5f * ShadowFactor * (difuseColor + specularColor)) * input.color ;

	return FrameBuffer;
}