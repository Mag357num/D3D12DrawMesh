//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

Texture2D shadowMap : register(t0);
SamplerState sampleClamp : register(s0);

#define SHADOW_DEPTH_BIAS 0.0002f

struct LightState
{
    float3 DirectionLightColor;
	float DirectionLightIntensity;
    float3 DirectionLightDir;
	float padding;
};

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 CameraVP;
	float4x4 LightVP;
    float4 CamEye;
	LightState Light;
	bool IsShadowPass;
};

float CalcUnshadowedAmountPCF2x2(float4 PosWorld)
{
    float4 LightSpacePos = PosWorld;
    LightSpacePos = mul(LightSpacePos, LightVP);

    LightSpacePos.xyz /= LightSpacePos.w;

    float2 TexCoord = 0.5f * LightSpacePos.xy + 0.5f;
    TexCoord.y = 1.0f - TexCoord.y;

    float LightSpaceDepth = LightSpacePos.z - SHADOW_DEPTH_BIAS;

	uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);
    float2 ShadowMapDims = float2(width, height);

    float4 SubPixelCoords = float4(1.0f, 1.0f, 1.0f, 1.0f);
    SubPixelCoords.xy = frac(ShadowMapDims * TexCoord);
    SubPixelCoords.zw = 1.0f - SubPixelCoords.xy;
    float4 BilinearWeights = SubPixelCoords.zxzx * SubPixelCoords.wwyy;

    float2 TexelUnits = 1.0f / ShadowMapDims;
    float4 ShadowDepths;
    ShadowDepths.x = shadowMap.Sample(sampleClamp, TexCoord);
    ShadowDepths.y = shadowMap.Sample(sampleClamp, TexCoord + float2(TexelUnits.x, 0.0f));
    ShadowDepths.z = shadowMap.Sample(sampleClamp, TexCoord + float2(0.0f, TexelUnits.y));
    ShadowDepths.w = shadowMap.Sample(sampleClamp, TexCoord + TexelUnits);

    float4 ShadowTests = (ShadowDepths > LightSpaceDepth) ? 1.0f : 0.0f;
    return dot(BilinearWeights, ShadowTests);
}

struct VSInput
{
    float3 position    : POSITION;
    float3 normal    : NORMAL;
    float2 uv0        : TEXCOORD0;
    float2 uv1        : TEXCOORD1;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldpos : POSITION;
	float3 normal : NORMAL;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.position = mul(float4(input.position, 1.0f), World);
	result.worldpos = result.position;
    result.position = mul(result.position, CameraVP);
    result.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	if(IsShadowPass)
	{
		return float4(1.f, 1.f, 1.f, 1.f);
	}

	input.normal = normalize(input.normal);
	float3 viewDir = CamEye.xyz - input.worldpos.xyz;
	viewDir = normalize(viewDir);
	float3 dir = normalize(Light.DirectionLightDir * -1.f);
	float3 halfWay = normalize(viewDir + dir);

	float ks = 1.5f;
	float shine = 10.f;
	float4 specularColor = ks * float4(Light.DirectionLightColor, 1.f) * pow(max(dot(input.normal, halfWay), 0.f), shine);

	float kd = 0.3f;
	float4 difuseColor = kd * float4(Light.DirectionLightColor, 1.f) * max(dot(input.normal, Light.DirectionLightDir.xyz * -1.f), 0.f);

	float ambientFactor = 0.02f;
	float4 ambientColor = ambientFactor * float4(Light.DirectionLightColor, 1.f);

	float4 Color = ambientColor + difuseColor + specularColor;
	float ShadowFactor = CalcUnshadowedAmountPCF2x2(input.worldpos);
	Color *= ShadowFactor;

	return float4(1.f, 1.f, 1.f, 1.f)*ShadowFactor;
}
