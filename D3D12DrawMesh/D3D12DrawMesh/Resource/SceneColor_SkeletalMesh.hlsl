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

cbuffer PaletteConstantBuffer : register(b4)
{
	float4x4 GBoneTransforms[68];
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
	uint4 BoneWeights : WEIGHTS;
	uint4 BoneIndices  : BONEINDICES;
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

	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = (float)input.BoneWeights.x / 255.f;
	weights[1] = (float)input.BoneWeights.y / 255.f;
	weights[2] = (float)input.BoneWeights.z / 255.f;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < 4; ++i)
	{
		posL += weights[i] * mul(float4(input.position, 1.0f), GBoneTransforms[input.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(input.normal, (float3x3)GBoneTransforms[input.BoneIndices[i]]);
	}

	input.position = posL;
	input.normal = normalL;

	result.position = mul(float4(input.position, 1.0f), World);
	result.worldpos = result.position;
	result.position = mul(result.worldpos, CameraVP);
	result.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
	result.color = float4(1.f, 1.f, 1.f, 1.f);
	result.shadowScreenPos = mul(result.worldpos, mul(VOMatrix, ScreenMatrix));

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float shine = 10.f;
	float4 SpecularColor = float4(0.f, 0.f, 0.f, 1.f);
	float4 DiffuseColor = float4(0.f, 0.f, 0.f, 1.f);
	float4 AmbientColor = float4(0.f, 0.f, 0.f, 1.f);

	float3 ViewDir = CamEye.xyz - input.worldpos.xyz;
	ViewDir = normalize(ViewDir);

	// directional light
	float3 DL_Dir = normalize(DirectionalLight.Dir * -1.f);
	float3 DL_HalfWay = normalize(ViewDir + DL_Dir);
	AmbientColor.xyz += DirectionalLight.Ambient;
	DiffuseColor.xyz += DirectionalLight.Diffuse * max(dot(input.normal, DL_Dir), 0.f);
	SpecularColor.xyz += DirectionalLight.Specular * pow(max(dot(input.normal, DL_HalfWay), 0.f), shine) * max(dot(input.normal, DL_Dir), 0.f); // multiple the dot(N, L) to avoid specular leak

	// point light
	for(int i = 0; i < 10; i++)
	{
		float3 PL_Dir = normalize( PointLight[i].State.Position - input.worldpos.xyz );
		float3 PL_HalfWay = normalize(ViewDir + PL_Dir);
		AmbientColor.xyz += PointLight[i].State.Ambient;
		DiffuseColor.xyz += PointLight[i].State.Diffuse * max(dot(input.normal, PL_Dir), 0.f);
		SpecularColor.xyz += PointLight[i].State.Specular * pow(max(dot(input.normal, PL_HalfWay), 0.f), shine) * max(dot(input.normal, PL_Dir), 0.f); // multiple the dot(N, L) to avoid specular leak
	}

	float bias = max(0.005f * (1.0f - abs(dot(input.normal, DirectionalLight.Dir))), 0.00005f);
	float ShadowFactor = CalcUnshadowedAmountPCF2x2(input.shadowScreenPos, bias);

	float4 FrameBuffer = (AmbientColor + 0.5f * ShadowFactor * (DiffuseColor + SpecularColor)) * input.color;

	return FrameBuffer;
}