#pragma once
#include "stdafx.h"
#include "Types.h"

struct FDirectionLight
{
	FVector Color = { 1, 1, 1 };
	float Intensity = 1.0f;

	FVector Dir = { -1, 0, 0 };
};

struct FPointLight
{
	FVector Color = { 1, 1, 1 };
	float Intensity = 1.0f;

	FVector Location = { 0, 0, 0 };
};