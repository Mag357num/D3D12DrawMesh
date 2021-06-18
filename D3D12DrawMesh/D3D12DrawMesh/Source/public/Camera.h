#pragma once
#include "stdafx.h"
#include "Key.h"
#include "Actor.h"
#include "DynamicRHI.h"

enum class FCameraMoveMode
{
	WANDER = 0,
	AROUNDTARGET = 1,
	STATIC = 2,
};

class FCameraComponent : public FActorComponent
{
private:
	// reset data
	FVector InitialEye;
	FVector InitialUp;
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

	// Secondary data, need to refresh depent on dirty
	bool VDirty = true;
	FMatrix VMatrix_GameThread;
	FMatrix VMatrix_RenderThread;
	bool PDirty = true;
	FMatrix PMatrix_GameThread;
	FMatrix PMatrix_RenderThread;

	shared_ptr<FStaticMeshComponent> CameraMesh;

public:
	FCameraComponent() = default;
	~FCameraComponent() = default;
	FCameraComponent(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f);

	void Reset();
	
	void SetScale(const FVector& Scale) { SetScale_Base(Scale); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetScale(Scale); } }
	void SetQuat(const FQuat& Quat) { SetQuat_Base(Quat); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetQuat(Quat); } }
	void SetTranslate(const FVector& Translate) { SetTranslate_Base(Translate); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetTranslate(Translate); } }
	void SetTransform(const FTransform& Trans) { SetTransform_Base(Trans); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetTransform(Trans); } }
	void SetWorldMatrix(const FMatrix& Matrix) { SetWorldMatrix_Base(Matrix); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetWorldMatrix(Matrix); } }
	void SetFov(const float& FovParam) { Fov = FovParam; PDirty = true; }
	void SetAspectRatio(const float& AspParam) { AspectRatio = AspParam; PDirty = true; }
	void SetViewPlane(const float& Near, const float& Far) { NearPlane = Near; FarPlane = Far; PDirty = true; }
	void SetLookAt(const FVector& Look);

	const FVector GetLookAt();
	const FMatrix& GetViewMatrix_GameThread();
	const FMatrix& GetViewMatrix_RenderThread();
	const FMatrix& GetPerspProjMatrix_GameThread();
	const FMatrix& GetPerspProjMatrix_RenderThread();
	FMatrix GetOrthoProjMatrix_GameThread(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f) const;
};

class ACameraActor : public AActor
{
private:
	FCameraComponent* const CameraComponent = Components[0]->As<FCameraComponent>();

	// movement parameter
	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	float AngularVelocity;

public:
	ACameraActor();
	ACameraActor(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f);
	~ACameraActor() = default;

	void SetMoveSpeed(const float& UnitsPerSecond);
	void SetTurnSpeed(const float& RadiansPerSecond);

	void Tick(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation = FVector(0.f, 0.f, 0.f), float Distance = 0.f);
	void UpdateCameraParam_Wander(const float& ElapsedSeconds);
	void UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, const FVector& TargetPos, const float& Distance);
	void UpdateCameraParam_Static(const float& ElapsedSeconds);

	FCameraComponent* GetRootComponent() { return CameraComponent; }
};