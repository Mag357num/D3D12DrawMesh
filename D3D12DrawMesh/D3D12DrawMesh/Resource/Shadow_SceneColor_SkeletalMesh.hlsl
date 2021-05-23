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
	float4x4 ShadowTransForm;
	float4x4 gBoneTransforms[68];
    float4 CamEye;
	LightState Light;
	bool IsShadowPass;
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
	float4 shadowPosH :POSITION1;
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
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

        posL += weights[i] * mul(float4(input.position, 1.0f), gBoneTransforms[input.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(input.normal, (float3x3)gBoneTransforms[input.BoneIndices[i]]);
    }

    input.position = posL;
    input.normal = normalL;

    result.position = mul(float4(input.position, 1.0f), World);
	result.worldpos = result.position;
    result.position = mul(result.worldpos, CameraVP);
    result.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
	result.color = float4(1.f, 1.f, 1.f, 1.f);

	if(!IsShadowPass)
	{
		result.shadowPosH = mul(result.worldpos, ShadowTransForm);
	}

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
	float4 specularColor;
	specularColor = ks *  float4(Light.DirectionLightColor, 1.f) * pow(max(dot(input.normal, halfWay), 0.f), shine);
	specularColor *= dot(input.normal, dir);

	float kd = 0.3f;
	float4 difuseColor = kd * float4(Light.DirectionLightColor, 1.f) * max(dot(input.normal, Light.DirectionLightDir.xyz * -1.f), 0.f);

	float ambientFactor = 0.02f;
	float4 ambientColor = ambientFactor * float4(Light.DirectionLightColor, 1.f);

	float bias = max(0.005f * (1.0f - abs(dot(input.normal, Light.DirectionLightDir))), 0.00005f);
	float ShadowFactor = CalcUnshadowedAmountPCF2x2(input.shadowPosH, bias);
	float4 FrameBuffer = (ambientColor + 0.5f * ShadowFactor * (difuseColor + specularColor)) * input.color ;

	return FrameBuffer;
}