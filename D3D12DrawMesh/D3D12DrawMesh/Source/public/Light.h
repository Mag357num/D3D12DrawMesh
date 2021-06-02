#pragma once
#include "stdafx.h"
#include "Camera.h"

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

class ALight : public AActor
{
//private:
//	FVector Color = { 1.f, 1.f, 1.f };
//	float Intensity = 1.0f;
//
//	bool VDirty = true;
//	FMatrix VMatrix;
//	bool PDirty = true;
//	FMatrix PMatrix;
//
//public:
//	const FVector& GetColor() const { return Color; }
//	const float& GetIntensity() const { return Intensity; }
//	void SetIntensity(const float& Inten) { Intensity = Inten; }
//	void SetColor(const FVector& C) { Color = C; }
};