#include "MathExtend.h"

#define PI (3.141592654f)
#define HALF_PI (1.5707963050f)
#define FLOAT_NON_FRACTIONAL (8388608.f) /* All single-precision floating point numbers greater than or equal to this have no fractional value. */
#define INV_PI (0.31830988618f)

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

FRotator QuatToEuler(FQuat Quat)
{
	const float SingularityTest = Quat.Z * Quat.X - Quat.W * Quat.Y;
	const float YawY = 2.f * (Quat.W * Quat.Z + Quat.X * Quat.Y);
	const float YawX = (1.f - 2.f * (std::pow(Quat.Y, 2) + std::pow(Quat.Z, 2)));

	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = (180.f) / PI;

	FRotator RotatorFromQuat;

	if (SingularityTest < -SINGULARITY_THRESHOLD)
	{
		RotatorFromQuat.Pitch = -90.f;
		RotatorFromQuat.Yaw = Atan2(YawY, YawX) * RAD_TO_DEG;
		RotatorFromQuat.Roll = NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * Atan2(Quat.X, Quat.W) * RAD_TO_DEG));
	}
	else if (SingularityTest > SINGULARITY_THRESHOLD)
	{
		RotatorFromQuat.Pitch = 90.f;
		RotatorFromQuat.Yaw = Atan2(YawY, YawX) * RAD_TO_DEG;
		RotatorFromQuat.Roll = NormalizeAxis(RotatorFromQuat.Yaw - (2.f * Atan2(Quat.X, Quat.W) * RAD_TO_DEG));
	}
	else
	{
		RotatorFromQuat.Pitch = FastAsin(2.f * (SingularityTest)) * RAD_TO_DEG;
		RotatorFromQuat.Yaw = Atan2(YawY, YawX) * RAD_TO_DEG;
		RotatorFromQuat.Roll = Atan2(-2.f * (Quat.W * Quat.X + Quat.Y * Quat.Z), (1.f - 2.f * (std::pow(Quat.X, 2) + std::pow(Quat.Y, 2)))) * RAD_TO_DEG;
	}

	return RotatorFromQuat;
}

FQuat EulerToQuat(FRotator Euler)
{
	const float DEG_TO_RAD = PI / (180.f);
	const float RADS_DIVIDED_BY_2 = DEG_TO_RAD / 2.f;
	float SP, SY, SR;
	float CP, CY, CR;

	const float PitchNoWinding = Fmod(Euler.Pitch, 360.0f);
	const float YawNoWinding = Fmod(Euler.Yaw, 360.0f);
	const float RollNoWinding = Fmod(Euler.Roll, 360.0f);

	SinCos(&SP, &CP, PitchNoWinding * RADS_DIVIDED_BY_2);
	SinCos(&SY, &CY, YawNoWinding * RADS_DIVIDED_BY_2);
	SinCos(&SR, &CR, RollNoWinding * RADS_DIVIDED_BY_2);

	FQuat RotationQuat;
	RotationQuat.X = CR * SP * SY - SR * CP * CY;
	RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
	RotationQuat.Z = CR * CP * SY - SR * SP * CY;
	RotationQuat.W = CR * CP * CY + SR * SP * SY;

	return RotationQuat;
}

float FastAsin(float Value)
{
	// Clamp input to [-1,1].
	bool nonnegative = (Value >= 0.0f);
	float x = std::abs(Value);
	float omx = 1.0f - x;
	if (omx < 0.0f)
	{
		omx = 0.0f;
	}
	float root = std::sqrt(omx);
	// 7-degree minimax approximation
	float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x + 0.0308918810f) * x - 0.0501743046f) * x + 0.0889789874f) * x - 0.2145988016f) * x + HALF_PI;
	result *= root;  // acos(|x|)
	// acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
	return (nonnegative ? HALF_PI - result : result - HALF_PI);
}

float NormalizeAxis(float Angle)
{
	// returns Angle in the range [0,360)
	Angle = ClampAxis(Angle);

	if (Angle > 180.f)
	{
		// shift to (-180,180]
		Angle -= 360.f;
	}

	return Angle;
}

float ClampAxis(float Angle)
{
	// returns Angle in the range (-360,360)
	Angle = Fmod(Angle, 360.f);

	if (Angle < 0.f)
	{
		// shift to [0,360) range
		Angle += 360.f;
	}

	return Angle;
}

float Fmod(float X, float Y)
{
	const float AbsY = fabsf(Y);
	if (AbsY <= 1.e-8f)
	{
		throw std::exception("cant mod zero");
		return 0.f;
	}
	const float Div = (X / Y);
	// All floats where abs(f) >= 2^23 (8388608) are whole numbers so do not need truncation, and avoid overflow in TruncToFloat as they get even larger.
	const float Quotient = fabsf(Div) < FLOAT_NON_FRACTIONAL ? truncf(Div) : Div;
	float IntPortion = Y * Quotient;

	// Rounding and imprecision could cause IntPortion to exceed X and cause the result to be outside the expected range.
	// For example Fmod(55.8, 9.3) would result in a very small negative value!
	if (fabsf(IntPortion) > fabsf(X))
	{
		IntPortion = X;
	}

	const float Result = X - IntPortion;
	// Clamp to [-AbsY, AbsY] because of possible failures for very large numbers (>1e10) due to precision loss.
	// We could instead fall back to stock fmodf() for large values, however this would diverge from the SIMD VectorMod() which has no similar fallback with reasonable performance.
	return Clamp(Result, -AbsY, AbsY);
}

void SinCos(float* ScalarSin, float* ScalarCos, float Value)
{
	// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = (INV_PI * 0.5f) * Value;
	if (Value >= 0.0f)
	{
		quotient = (float)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = Value - (2.0f * PI) * quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
	float sign;
	if (y > HALF_PI)
	{
		y = PI - y;
		sign = -1.0f;
	}
	else if (y < -HALF_PI)
	{
		y = -PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = +1.0f;
	}

	float y2 = y * y;

	// 11-degree minimax approximation
	*ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

	// 10-degree minimax approximation
	float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
	*ScalarCos = sign * p;
}