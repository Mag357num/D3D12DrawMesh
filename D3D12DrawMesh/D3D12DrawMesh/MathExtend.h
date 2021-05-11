#pragma once
#include "stdafx.h"

#include <glm.hpp>
#include <ext/vector_float3.hpp>
#include <ext/matrix_float4x4.hpp>

using FVector2 = glm::vec2;
using FVector = glm::vec3;
using FVector4 = glm::vec4;
using FMatrix = glm::mat4;

struct FRotator // order same with ue4
{
	float Roll;
	float Pitch;
	float Yaw;
};

struct FTransform
{
	FVector Translation;
	FRotator Rotation;
	FVector Scale;
};

float Atan2(const float& Y, const float& X);

FVector4 GetBufferSizeAndInvSize(FVector2 Param);

FVector QuatToEuler(FVector4 Quat);