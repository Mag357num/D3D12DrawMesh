#pragma once
#include "stdafx.h"

#include <glm.hpp>
#include <ext/vector_float3.hpp>
#include <ext/matrix_float4x4.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/euler_angles.hpp>
#include <gtx/transform.hpp>

using FVector2 = glm::vec2;
using FVector = glm::vec3;
using FVector4 = glm::vec4;
using FMatrix = glm::mat4; // FMatrix[column][row]
using FQuat = glm::tquat<float>;

using namespace glm;

// projection space to screen space transform
static FMatrix ProjToScreen(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f);

static float Pi = 3.141592654f;
static float Inv_Pi = 0.31830988618f;
static float Half_Pi = 1.5707963050f;
static float Float_Non_Fractional = 8388608.f;

struct FTransform
{
	FVector Scale;
	FQuat Quat;
	FVector Translation;
};

struct FEuler
{
	FEuler() = default;
	FEuler(float Roll, float Pitch, float Yaw) : Roll(Roll), Pitch(Pitch), Yaw(Yaw) {}

	float Roll;
	float Pitch;
	float Yaw;
};

FVector QuatToLook(const FQuat& Quat);
FQuat LookToQuat(const FVector& Look);
FEuler QuatToEuler(const FQuat& Quat);
FQuat EulerToQuat(const FEuler& Euler);

//float Atan2(const float& Y, const float& X);

FVector4 GetBufferSizeAndInvSize(FVector2 Param);


//FQuat CalFQuat(FVector dir)
//{
//	FQuat cal = new FQuat();
//	FVector euler = FQuat.LookRotation(dir).eulerAngles;
//
//	//euler Y: cosY = z/sqrt(x^2+z^2)
//	float CosY = dir.z / Mathf.Sqrt(dir.x * dir.x + dir.z * dir.z);
//	float CosYDiv2 = Mathf.Sqrt((CosY + 1) / 2);
//	if (dir.x < 0) CosYDiv2 = -CosYDiv2;
//
//	float SinYDiv2 = Mathf.Sqrt((1 - CosY) / 2);
//
//	//euler X: cosX = sqrt((x^2+z^2)/(x^2+y^2+z^2)
//	float CosX = Mathf.Sqrt((dir.x * dir.x + dir.z * dir.z) / (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z));
//	if (dir.z < 0) CosX = -CosX;
//	float CosXDiv2 = Mathf.Sqrt((CosX + 1) / 2);
//	if (dir.y > 0) CosXDiv2 = -CosXDiv2;
//	float SinXDiv2 = Mathf.Sqrt((1 - CosX) / 2);
//
//	//quat w = cos(x/2)cos(y/2)
//	cal.w = CosXDiv2 * CosYDiv2;
//	//quat x = sin(x/2)cos(y/2)
//	cal.x = SinXDiv2 * CosYDiv2;
//	//quat y = cos(x/2)sin(y/2)
//	cal.y = CosXDiv2 * SinYDiv2;
//	//quat z = sin(x/2)sin(y/2)
//	cal.z = -SinXDiv2 * SinYDiv2;
//
//
//	CalCosX = CosX;
//	CalCosY = CosY;
//	RightCosX = Mathf.Cos(Mathf.Deg2Rad * (FQuat.LookRotation(dir).eulerAngles.x));
//	RightCosY = Mathf.Cos(Mathf.Deg2Rad * (FQuat.LookRotation(dir).eulerAngles.y));
//	RightEulers = FQuat.LookRotation(dir).eulerAngles;
//
//	return cal;
//}
