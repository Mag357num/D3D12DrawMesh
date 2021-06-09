Texture2D shadowMap : register(t0);
SamplerState sampleClamp : register(s0);

static const float4x4 ScreenMatrix = float4x4(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f);

struct DirectionalLightState
{
	float3 Dir;
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
};

struct PointLightState
{
	float3 Position;
	float3 Ambient;
	float3 Diffuse;
	float3 Specular;
	float3 Attenuation;
};

struct PointLightData
{
	float4x4 VPMatrix[6];
	PointLightState State;
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
	float4x4 VOMatrix;
	DirectionalLightState DirectionalLight;
};

cbuffer PLightConstantBuffer : register(b3)
{
	PointLightData PointLight[10]; // TODO: hard code as 10
};

float CalcUnshadowedAmountPCF2x2(float4 ScreenSpacePos, float ShadowBias)
{
	float2 TexCoord = ScreenSpacePos.xy;
	float ActualDepth = ScreenSpacePos.z - ShadowBias;

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
	float4 ShadowScreenPos :POSITION1;
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
	result.ShadowScreenPos = mul(result.worldpos, mul(VOMatrix, ScreenMatrix));

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float shine = 10.f;

	float3 ViewDir = CamEye.xyz - input.worldpos.xyz;
	ViewDir = normalize(ViewDir);

	// directional light
	float3 DL_Dir = normalize(DirectionalLight.Dir * -1.f);
	float3 DL_HalfWay = normalize(ViewDir + DL_Dir);
	float3 DL_AmbientColor = DirectionalLight.Ambient;
	float3 DL_DiffuseColor = DirectionalLight.Diffuse * max(dot(input.normal, DL_Dir), 0.f);
	float3 DL_SpecularColor = DirectionalLight.Specular * pow(max(dot(input.normal, DL_HalfWay), 0.f), shine) * max(dot(input.normal, DL_Dir), 0.f); // multiple the dot(N, L) to avoid specular leak

	// directional light shadow
	float ShadowBias = max(0.005f * (1.0f - abs(dot(input.normal, DirectionalLight.Dir))), 0.00005f);
	float ShadowFactor = CalcUnshadowedAmountPCF2x2(input.ShadowScreenPos, ShadowBias);

	float4 FrameBuffer = float4((DL_AmbientColor + 0.5f * ShadowFactor * (DL_DiffuseColor + DL_SpecularColor)), 1.f) * input.color;

	// point light
	for(int i = 0; i < 10; i++)
	{
		float3 PL_Dir = normalize( PointLight[i].State.Position - input.worldpos.xyz );
		float3 PL_HalfWay = normalize(ViewDir + PL_Dir);
		float3 PL_AmbientColor = PointLight[i].State.Ambient;
		float3 PL_DiffuseColor = PointLight[i].State.Diffuse * max(dot(input.normal, PL_Dir), 0.f);
		float3 PL_SpecularColor = PointLight[i].State.Specular * pow(max(dot(input.normal, PL_HalfWay), 0.f), shine) * max(dot(input.normal, PL_Dir), 0.f); // multiple the dot(N, L) to avoid specular leak
		FrameBuffer += float4((PL_AmbientColor + 0.5f * (PL_DiffuseColor + PL_SpecularColor)), 1.f) * input.color;
	}

	return FrameBuffer;
}