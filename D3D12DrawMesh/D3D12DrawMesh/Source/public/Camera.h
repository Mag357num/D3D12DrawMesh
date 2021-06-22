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

class FCameraComponent : public FSceneComponent
{
private:
	// reset data
	FVector InitialEye;
	FVector InitialUp;
	FVector InitialLookAt;

	// mode
	uint32 ProjectionMode; // 0:persp, 1: ortho

	// perspective frustum parameter
	float Fov = 90.f;
	float AspectRatio = 1.7777777f;

	// ortho frustum parameter
	float OrthoWidth = 512.f;

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
	bool ODirty = true;
	FMatrix OMatrix_GameThread;
	FMatrix OMatrix_RenderThread;

	shared_ptr<FStaticMeshComponent> CameraMesh;

public:
	friend class ACameraActor;
	FCameraComponent() = default;
	~FCameraComponent() = default;
	FCameraComponent(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.0f, const float& FarPlane = 5000.0f);

	void Reset();
	
	void SetScale(const FVector& Scale) { SetScale_Base(Scale); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetScale(Scale); } }
	void SetQuat(const FQuat& Quat) { SetQuat_Base(Quat); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetQuat(Quat); } }
	void SetTranslate(const FVector& Translate) { SetTranslate_Base(Translate); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetTranslate(Translate); } }
	void SetTransform(const FTransform& Trans) { SetTransform_Base(Trans); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetTransform(Trans); } }
	void SetWorldMatrix(const FMatrix& Matrix) { SetWorldMatrix_Base(Matrix); VDirty = true; if (CameraMesh.get() != nullptr) { CameraMesh->SetWorldMatrix(Matrix); } }
	void SetProjectMode(const uint32& Mode) { ProjectionMode = Mode; }
	void SetFov(const float& FovParam) { Fov = FovParam; PDirty = true; }
	void SetOrthoWidth(const float& Width) { OrthoWidth = Width; }
	void SetAspectRatio(const float& AspParam) { AspectRatio = AspParam; PDirty = true; }
	void SetViewPlane(const float& Near, const float& Far) { NearPlane = Near; FarPlane = Far; PDirty = true; }
	void SetLookAt(const FVector& Look);

	FStaticMeshComponent* GetCameraMesh() { return CameraMesh.get(); }
	const FVector GetLookAt();
	const uint32& GetProjectMode() { return ProjectionMode; }
	const FMatrix& GetViewMatrix_GameThread();
	const FMatrix& GetViewMatrix_RenderThread();
	const FMatrix& GetPerspProjMatrix_GameThread();
	const FMatrix& GetPerspProjMatrix_RenderThread();
	const FMatrix& GetOrthoProjMatrix_GameThread();
	const FMatrix& GetOrthoProjMatrix_RenderThread();

};

class ACameraActor : public AActor
{
private:
	// movement parameter
	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	float AngularVelocity;

	FCameraComponent* CameraComponent;

public:
	ACameraActor(shared_ptr<FCameraComponent> Cam) { RootComponent = Cam; CameraComponent = Cam.get(); OwnedComponents.push_back(Cam); }

	void SetMoveSpeed(const float& UnitsPerSecond);
	void SetTurnSpeed(const float& RadiansPerSecond);
	void SetCameraMesh(const shared_ptr<FStaticMeshComponent> Mesh) { CameraComponent->CameraMesh = Mesh; OwnedComponents.push_back(Mesh); }

	FCameraComponent* GetCameraComponent() { return CameraComponent; }

	void Tick(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation = FVector(0.f, 0.f, 0.f), float Distance = 0.f);
	void UpdateCameraParam_Wander(const float& ElapsedSeconds);
	void UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, const FVector& TargetPos, const float& Distance);
	void UpdateCameraParam_Static(const float& ElapsedSeconds);
};