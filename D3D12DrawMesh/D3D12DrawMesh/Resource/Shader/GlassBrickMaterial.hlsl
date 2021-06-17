Texture2D shadowMap : register(t0);
Texture2D BaseColorMap : register(t1);
SamplerState sampleClamp : register(s0);

static const float4x4 ScreenMatrix = float4x4(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f);

struct PixelMaterialInputs
{
	float4 EmissiveColor;
	float Opacity;
	float OpacityMask;
	float4 BaseColor;
	float Metallic;
	float Specular;
	float Roughness;
	float Anisotropy;
	float3 Normal;
	float3 Tangent;
	float Subsurface;
	float AmbientOcclusion;
	float2 Refraction;
	float PixelDepthOffset;
	float ShadingModel;
};

struct MaterialParamInstance
{
	// parameter in this constantbuffer depends on what parameter material need
	float Opacity;
};

PixelMaterialInputs CalcPixelMaterialInputs(MaterialParamInstance Param, float2 uv)
{
	PixelMaterialInputs Inputs;

	// code here depend on the node graph in ue4 material editor
	{
		Inputs.BaseColor = BaseColorMap.Sample(sampleClamp, uv);
		Inputs.Opacity = Param.Opacity;
	}

	return Inputs;
}

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

cbuffer MaterialParameter : register(b4)
{
	MaterialParamInstance MaterialParams;
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
	float2 uv0        : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
	PSInput result;
	result.position = mul(float4(input.position, 1.0f), World);
	result.worldpos = result.position;
	result.position = mul(result.worldpos, CameraVP);
	result.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
	result.ShadowScreenPos = mul(result.worldpos, mul(VOMatrix, ScreenMatrix));
	result.uv0 = input.uv0;
	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	// material inputs
	PixelMaterialInputs MaterialInputs = CalcPixelMaterialInputs(MaterialParams, input.uv0);

	// temporary static parameter
	float Shine = 1.f;

	// common parameter
	float ShadowBias = max(0.005f * (1.0f - abs(dot(input.normal, DirectionalLight.Dir))), 0.00005f);
	float3 ViewDir = normalize(CamEye.xyz - input.worldpos.xyz);
	float4 FrameBuffer = float4(0.f, 0.f, 0.f, 1.f);

	// directional light
	float3 DL_Dir = normalize(DirectionalLight.Dir * -1.f);
	float3 DL_HalfWay = normalize(ViewDir + DL_Dir);
	float3 DL_Ambient = DirectionalLight.Ambient;
	float3 DL_Diffuse = DirectionalLight.Diffuse * max(dot(input.normal, DL_Dir), 0.f);
	float3 DL_Specular = DirectionalLight.Specular * pow(max(dot(input.normal, DL_HalfWay), 0.f), Shine) * max(dot(input.normal, DL_Dir), 0.f); // multiple the dot(N, L) to avoid specular leak

	// directional light shadow
	{
		float ShadowFactor = CalcUnshadowedAmountPCF2x2(input.ShadowScreenPos, ShadowBias);
		FrameBuffer += float4(DL_Ambient + 0.5f * ShadowFactor * (DL_Diffuse + DL_Specular), 0.f) * MaterialInputs.BaseColor;
	}

	// point light
	for(int i = 0; i < 10; i++)
	{
		float3 PL_Dir = normalize( PointLight[i].State.Position - input.worldpos.xyz );
		float3 PL_HalfWay = normalize(ViewDir + PL_Dir);
		float3 PL_Ambient = PointLight[i].State.Ambient;
		float3 PL_Diffuse = PointLight[i].State.Diffuse * max(dot(input.normal, PL_Dir), 0.f);
		float3 PL_Specular = PointLight[i].State.Specular * pow(max(dot(input.normal, PL_HalfWay), 0.f), Shine) * max(dot(input.normal, PL_Dir), 0.f); // multiple the dot(N, L) to avoid specular leak
		
		// point light shadow
		float ShadowFactor = 1.f;
		FrameBuffer += float4(PL_Ambient + 0.5f * ShadowFactor * (PL_Diffuse + PL_Specular), 0.f) * MaterialInputs.BaseColor;
	}

	FrameBuffer.w *= MaterialInputs.Opacity;

	return FrameBuffer;
}