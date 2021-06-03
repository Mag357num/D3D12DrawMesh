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
	// reset data
	FVector InitialPosition;
	FVector InitialUpDir;
	FVector InitialLookAt;

	// perspective frustum parameter
	float Fov = 90.f;
	float AspectRatio = 1.7777777f;

	// ortho frustum parameter
	float Left;
	float Right;
	float Bottom;
	float Top;

	// common parameter
	float NearPlane = 1.f;
	float FarPlane = 5000.f;

	// movement parameter
	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	float AngularVelocity;

	// data
	bool VDirty = true;
	FMatrix VMatrix_GameThread;
	FMatrix VMatrix_RenderThread;
	bool PDirty = true;
	FMatrix PMatrix_GameThread;
	FMatrix PMatrix_RenderThread;

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

	void SetLookAt(const FVector& Look);
	const FVector GetLookAt();

	const bool& IsVDirty() const { return VDirty; }
	const bool& IsPDirty() const { return PDirty; }

	void Tick(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation = FVector(0.f, 0.f, 0.f), float Distance = 0.f);
	void UpdateCameraParam_Wander(const float& ElapsedSeconds);
	void UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, const FVector& TargetPos, const float& Distance);
	void UpdateCameraParam_Static(const float& ElapsedSeconds);

	const FMatrix& GetViewMatrix_GameThread();
	const FMatrix& GetViewMatrix_RenderThread();

	const FMatrix& GetPerspProjMatrix_GameThread();
	const FMatrix& GetPerspProjMatrix_RenderThread();

	FMatrix GetOrthoProjMatrix_GameThread(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f) const;

	void Reset();
};