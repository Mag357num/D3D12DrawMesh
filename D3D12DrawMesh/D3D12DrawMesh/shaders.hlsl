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

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 WVP;
    float4x4 World;
    float4x4 Rotator;
    float3 CamEye;
    float3 DirectionLightDir;
    float3 DirectionLightColor;
	float DirectionLightIntensity;
};

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

    result.position = mul(float4(input.position, 1.0f), WVP);
	result.worldpos = mul(float4(input.position, 1.0f), World);
    result.normal = mul(float4(input.normal, 1.0f), Rotator).xyz;


	result.color = float4((input.normal + 1) / 2, 1.0f);
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float3 viewDir = CamEye - input.worldpos;
	float3 halfWay = normalize(viewDir + DirectionLightDir * -1.f);

	float ks = 0.0f;
	float shine = 10.f;
	float4 specularColor = ks * float4(DirectionLightColor, 1.f) * pow(max(dot(input.normal, halfWay), 0.0), shine);

	float kd = 0.0f;
	float difuseColor = kd * float4(DirectionLightColor, 1.f) * max(dot(input.normal, DirectionLightDir * -1.f), 0.0);

	float ambientFactor = 0.1f;
	float4 ambientColor = ambientFactor * float4(DirectionLightColor, 1.f);

	float Color = ambientColor + difuseColor + specularColor;

	return Color;
}
