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

struct FQuat
{
	float X;
	float Y;
	float Z;
	float W;
};

struct FTransform
{
	FVector Scale;
	FRotator Rotation;
	FVector Translation;
};

float Atan2(const float& Y, const float& X);

FVector4 GetBufferSizeAndInvSize(FVector2 Param);

FRotator QuatToEuler(FQuat Quat);

FQuat EulerToQuat(FRotator Euler);

float FastAsin(float Value);

float NormalizeAxis(float Angle);

float ClampAxis(float Angle);

float Fmod(float X, float Y);

void SinCos(float* ScalarSin, float* ScalarCos, float Value);

template< class T >
T Clamp(const T X, const T Min, const T Max)
{
	return X < Min ? Min : X < Max ? X : Max;
}