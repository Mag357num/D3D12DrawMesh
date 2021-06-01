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
public:
	// if camera parameter changed, need to update constant buffers related to camera parameter
	bool IsChanged = true;

private:
	// the backup date of init/reset camera
	FVector InitialPosition;
	FVector InitialUpDir;
	FVector InitialLookAt;

	// the parameter to determine a camera
	FVector Position;
	FVector LookDirection;
	FVector UpDirection = { 0.f, 0.f, 1.f }; // UpDirection is the up direction in world space
	float Fov = 90.f;
	float AspectRatio = 1.7777777f;

	// camera movement variable
	float MoveSpeed = 300.0f; // Speed at which the camera moves, in units per second.
	float TurnSpeed = 1.570796327f; // Speed at which the camera turns, in radians per second.
	float MouseSensibility = 0.005f;

private:
	class FStaticMeshComponent* GetStaticMeshCom();

public:
	ACamera();
	~ACamera() = default;
	ACamera( const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float Width, float Height );

	void Init(const FVector& Eye, const FVector& Up, const FVector& LookAt, float Fov, float Width, float Height);
	void Update(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation = FVector(0, 0, 0), float Distance = 0.f);
	void UpdateCameraParam_Wander(const float& ElapsedSeconds);
	void UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, FVector TargetLocation, float Distance);
	void UpdateCameraParam_Static(const float& ElapsedSeconds);

	FMatrix GetViewMatrix() const;
	FMatrix GetPerspProjMatrix(const float& NearPlane = 1.0f, const float& FarPlane = 1000.0f) const;
	FMatrix GetOrthoProjMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top ,const float& NearPlane = 1.0f, const float& FarPlane = 1000.0f) const;
	FVector GetPosition() { return Position; }
	FVector GetLook() { return LookDirection; }
	FVector2 GetEulerByLook( const FVector& LookAt );
	void UpdateLookByEuler( const float& Pitch, const float& Yaw );

	void SetMoveSpeed( const float& UnitsPerSecond );
	void SetTurnSpeed( const float& RadiansPerSecond );
	void SetFov(const float& FovParam) { Fov = FovParam; }
	void SetAspectRatio(const float& AspParam) { AspectRatio = AspParam; }

	void Reset();
};