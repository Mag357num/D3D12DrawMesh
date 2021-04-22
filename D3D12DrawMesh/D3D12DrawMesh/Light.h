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
	FVector Dir = { -1.f, 0.f, 0.f };
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