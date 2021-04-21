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
    result.normal = normalize(mul(float4(input.normal, 1.0f), Rotator).xyz);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float3 viewDir = CamEye - input.worldpos.xyz;
	viewDir = normalize(viewDir);
	float3 dir = normalize(DirectionLightDir * -1.f);
	float3 halfWay = normalize(viewDir + dir);

	float ks = 15.f;
	float shine = 10.f;
	float4 specularColor = ks * float4(DirectionLightColor, 1.f) * pow(max(dot(input.normal, halfWay), 0.f), shine);

	float kd = 0.0f;
	float4 difuseColor = kd * float4(DirectionLightColor, 1.f) * max(dot(input.normal, DirectionLightDir * -1.f), 0.f);

	float ambientFactor = 0.0f;
	float4 ambientColor = ambientFactor * float4(DirectionLightColor, 1.f);

	float4 Color = ambientColor + difuseColor + specularColor;

	return Color;
}
