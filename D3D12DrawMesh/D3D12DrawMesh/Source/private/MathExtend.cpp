#include "MathExtend.h"

#define PI (3.141592654f)
#define HALF_PI (1.5707963050f)
#define FLOAT_NON_FRACTIONAL (8388608.f) /* All single-precision floating point numbers greater than or equal to this have no fractional value. */
#define INV_PI (0.31830988618f)

FVector QuatToLook(const FQuat& Quat)
{
	FMatrix MQ = glm::toMat4(Quat);
	FVector4 Look4 = FVector4(0, 1, 0, 1) * MQ; // regard (0, 1, 0) as the init look dir because model face (0, 1, 0) when there are no rotate
	return FVector(Look4.x, Look4.y, Look4.z);
}

FQuat LookToQuat(const FVector& Look)
{
	return FQuat(0.f, 0.f, 0.f, 1.f);
}

FEuler QuatToEuler(const FQuat& Quat)
{
	// euler use in right hand coord, xyz: pitch yaw roll
	FVector RightHandEuler = glm::eulerAngles(Quat);

	// euler use in left hand coord, xyz: roll pitch yaw 
	FEuler LeftHandEuler;
	LeftHandEuler.Roll = RightHandEuler.x;
	LeftHandEuler.Pitch = RightHandEuler.y;
	LeftHandEuler.Yaw = RightHandEuler.z;

	return LeftHandEuler;
}

FQuat EulerToQuat(const FEuler& Euler)
{
	return FQuat(FVector(Euler.Roll, Euler.Pitch, Euler.Yaw));
}

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