#include "MathExtend.h"

float PI = 3.141592654f;

float Atan2(const float& Y, const float& X)
{
	const float absX = fabs(X);
	const float absY = fabs(Y);
	const bool yAbsBigger = (absY > absX);
	float t0 = yAbsBigger ? absY : absX; // Max(absY, absX)
	float t1 = yAbsBigger ? absX : absY; // Min(absX, absY)

	if (t0 == 0.f)
		return 0.f;

	float t3 = t1 / t0;
	float t4 = t3 * t3;

	static const float c[7] = {
		+7.2128853633444123e-03f,
		-3.5059680836411644e-02f,
		+8.1675882859940430e-02f,
		-1.3374657325451267e-01f,
		+1.9856563505717162e-01f,
		-3.3324998579202170e-01f,
		+1.0f
	};

	t0 = c[0];
	t0 = t0 * t4 + c[1];
	t0 = t0 * t4 + c[2];
	t0 = t0 * t4 + c[3];
	t0 = t0 * t4 + c[4];
	t0 = t0 * t4 + c[5];
	t0 = t0 * t4 + c[6];
	t3 = t0 * t3;

	t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
	t3 = (X < 0.0f) ? PI - t3 : t3;
	t3 = (Y < 0.0f) ? -t3 : t3;

	return t3;
}

FVector4 GetBufferSizeAndInvSize(FVector2 Param)
{
	return FVector4(std::ceil(Param.x), std::ceil(Param.y), 1.f / std::ceil(Param.x), 1.f / std::ceil(Param.y));
}

FVector QuatToEuler(FVector4 Quat) // copy from ue4
{
	const float SingularityTest = Quat.z * Quat.x - Quat.w * Quat.y;
	const float YawY = 2.f * (Quat.w * Quat.z + Quat.x * Quat.y);
	const float YawX = (1.f - 2.f * (std::pow(Quat.y, 2) + std::pow(Quat.z, 2)));

	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = (180.f) / PI;

	FVector RotatorFromQuat(1, 1, 1);
	//FVector RotatorFromQuat;

	//if (SingularityTest < -SINGULARITY_THRESHOLD)
	//{
	//	RotatorFromQuat.Pitch = -90.f;
	//	RotatorFromQuat.Yaw = Atan2(YawY, YawX) * RAD_TO_DEG;
	//	RotatorFromQuat.Roll = FRotator::NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * Atan2(Quat.x, Quat.w) * RAD_TO_DEG));
	//}
	//else if (SingularityTest > SINGULARITY_THRESHOLD)
	//{
	//	RotatorFromQuat.Pitch = 90.f;
	//	RotatorFromQuat.Yaw = Atan2(YawY, YawX) * RAD_TO_DEG;
	//	RotatorFromQuat.Roll = FRotator::NormalizeAxis(RotatorFromQuat.Yaw - (2.f * Atan2(Quat.x, Quat.w) * RAD_TO_DEG));
	//}
	//else
	//{
	//	RotatorFromQuat.Pitch = FMath::FastAsin(2.f * (SingularityTest)) * RAD_TO_DEG;
	//	RotatorFromQuat.Yaw = Atan2(YawY, YawX) * RAD_TO_DEG;
	//	RotatorFromQuat.Roll = Atan2(-2.f * (Quat.w * Quat.x + Quat.y * Quat.z), (1.f - 2.f * (FMath::Square(Quat.x) + FMath::Square(Quat.y)))) * RAD_TO_DEG;
	//}

	return RotatorFromQuat;
}