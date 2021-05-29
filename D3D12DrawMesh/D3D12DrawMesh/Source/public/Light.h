#pragma once
#include "stdafx.h"

struct FLight
{
	FVector Color = { 1.f, 1.f, 1.f };
	float Intensity = 1.0f;
};

struct FDirectionLight : public FLight
{
	FVector Dir = { -1.f, 0.f, -1.f };
};

struct FPointLight : public FLight
{
	FVector Position = { 0.f, 0.f, 0.f };
	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;
};