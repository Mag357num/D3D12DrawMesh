#pragma once
#include "stdafx.h"
#include "Key.h"
#include "Actor.h"

enum class FCameraMoveMode
{
	WANDER = 0,
	AROUNDTARGET = 1,
	STATIC = 2,
};

class ACamera : public AActor
{
private:
	FVector InitialPosition;
	FVector InitialUpDir;
	FVector InitialLookAt;

	float Fov = 90.f;
	float AspectRatio = 1.7777777f;
	float NearPlane = 1.f;
	float FarPlane = 5000.f;

	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	//float AngularVelocity;

	bool VDirty = true;
	FMatrix VMatrix;
	bool PDirty = true;
	FMatrix PMatrix;

public:
	ACamera();
	~ACamera() = default;
	ACamera(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f);

	void Init(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f);
	
	void SetMoveSpeed(const float& UnitsPerSecond);
	void SetTurnSpeed(const float& RadiansPerSecond);

	void SetFov(const float& FovParam) { Fov = FovParam; PDirty = true; }
	void SetAspectRatio(const float& AspParam) { AspectRatio = AspParam; PDirty = true; }
	void SetViewPlane(const float& Near, const float& Far) { NearPlane = Near; FarPlane = Far; PDirty = true; }

	void SetQuat(const FQuat& Quat);
	void SetTranslate(const FVector& Trans);
	void SetWorldMatrix(const FMatrix& W);
	const FTransform& GetTransform();
	const FMatrix& GetWorldMatrix();

	const bool& IsVDirty() const { return VDirty; }
	const bool& IsPDirty() const { return PDirty; }

	void Tick(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation = FVector(0, 0, 0), float Distance = 0.f);
	void UpdateCameraParam_Wander(const float& ElapsedSeconds);
	void UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, const FVector& TargetPos, const float& Distance);
	void UpdateCameraParam_Static(const float& ElapsedSeconds);

	FMatrix GetViewMatrix();
	FMatrix GetPerspProjMatrix();
	FMatrix GetOrthoProjMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f) const;

	void Reset();
};