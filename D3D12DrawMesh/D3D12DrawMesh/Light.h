#pragma once
#include "stdafx.h"
#include "Types.h"

struct FBaseLight
{
	FVector Color = { 1.f, 1.f, 1.f };
	float Intensity = 1.0f;
};

struct FDirectionLight : public FBaseLight
{
	// CamEye and DirectionLightDir use FVector4 but DirectionLightColor use FVector is becuase of the hlsl packing rules
	// that is hlsl variable cant straddle between two float4, if use FVector, some data may cant read in hlsl
	FVector4 Dir = { -1.f, 0.f, 0.f, 0.f };
};

struct FPointLight : public FBaseLight
{
	FVector Position = { 0.f, 0.f, 0.f };

	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;

};