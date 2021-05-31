#pragma once
#include "stdafx.h"

struct FLight
{
	FVector Position = { 450.f, 0.f, 450.f };
	bool IsChanged = true;
	FVector Color = { 1.f, 1.f, 1.f };
	float Intensity = 1.0f;
};

struct FDirectionLight : public FLight
{
	FVector Dir = { -1.f, 0.f, -1.f };
	FMatrix VPMatrix;

	FMatrix GetLightVPMatrix();
};

struct FPointLight : public FLight
{
	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;
};