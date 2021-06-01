#include "Camera.h"
#include "StaticMesh.h"
#include "DeviceEventProcessor.h"

ACamera::ACamera()
{
	Components.push_back(make_shared<FStaticMeshComponent>());
}

FStaticMeshComponent* ACamera::GetStaticMeshCom()
{
	if (Components.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return Components[0].get()->As<FStaticMeshComponent>();
	}
}

ACamera::ACamera( const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float Width, float Height )
{
	Components.push_back(make_shared<FStaticMeshComponent>());
	Init( PositionParam, UpDir, LookAt, Fov, Width, Height );
}

void ACamera::UpdateLookByEuler(const float& Pitch, const float& Yaw)
{
	LookDirection.x = cosf(Pitch) * cosf(Yaw);
	LookDirection.y = cosf(Pitch) * sinf(Yaw);
	LookDirection.z = sinf(Pitch);

	if (fabs(LookDirection.x) < 0.001f) LookDirection.x = 0.f;
	if (fabs(LookDirection.y) < 0.001f) LookDirection.y = 0.f;
	if (fabs(LookDirection.z) < 0.001f) LookDirection.z = 0.f;
}

void ACamera::Init(const FVector& Eye, const FVector& Up, const FVector& LookAt, float Fov, float Width, float Height)
{
	InitialPosition = Eye;
	InitialUpDir = Up;
	InitialLookAt = LookAt;

	// eye, look, up construct a view matrix and view matrix is inverse matrix of camera entity
	FMatrix ComponentMatrix = glm::inverse(glm::lookAtLH(Eye, Eye + LookAt * 10.0f, Up));
	GetStaticMeshCom()->SetMatrix(ComponentMatrix);

	Position = Eye;
	UpDirection = Up;
	LookDirection = LookAt;

	SetFov(Fov);
	SetAspectRatio(Width / Height);
}

void ACamera::SetMoveSpeed(const float & UnitsPerSecond)
{
	MoveSpeed = UnitsPerSecond;
}

void ACamera::SetTurnSpeed(const float& RadiansPerSecond)
{
	TurnSpeed = RadiansPerSecond;
}

void ACamera::Reset()
{
	Position = InitialPosition;
	UpDirection = InitialUpDir;
	LookDirection = InitialLookAt;
}

void ACamera::UpdateCameraParam_Wander(const float& ElapsedSeconds)
{
	const KeysPressed& Keys = FDeviceEventProcessor::Get()->GetKeys();
	const FVector2& MouseMoveDelta = FDeviceEventProcessor::Get()->GetDeltaMouseMove_BottonDown();
	const bool& bIsMouseDown = FDeviceEventProcessor::Get()->IsMouseDown();

	// Interval between tick 
	float KeyKeyMoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;
	FVector2 MouseRotateInterval = MouseSensibility * MouseMoveDelta;

	FVector2 YawPitch = GetEulerByLook(LookDirection);
	float& Yaw = YawPitch.x;
	float& Pitch = YawPitch.y;

	// wasd for movement
	FVector Move(0.f, 0.f, 0.f);
	if (Keys.w)
	{
		Move.x += KeyKeyMoveInterval * cos(Pitch);
		Move.z += KeyKeyMoveInterval * sin(Pitch);
	}
	if (Keys.s)
	{
		Move.x -= KeyKeyMoveInterval * cos(Pitch);
		Move.z -= KeyKeyMoveInterval * sin(Pitch);
	}
	if (Keys.d)
	{
		Move.y += KeyKeyMoveInterval;
	}
	if (Keys.a)
	{
		Move.y -= KeyKeyMoveInterval;
	}

	// up down left right for movement
	if (Keys.up)
	{
		Move.x += KeyKeyMoveInterval * cos(Pitch);
		Move.z += KeyKeyMoveInterval * sin(Pitch);
	}
	if (Keys.down)
	{
		Move.x -= KeyKeyMoveInterval * cos(Pitch);
		Move.z -= KeyKeyMoveInterval * sin(Pitch);
	}
	if (Keys.right)
	{
		Move.y += KeyKeyMoveInterval;
	}
	if (Keys.left)
	{
		Move.y -= KeyKeyMoveInterval;
	}

	// qe for move up and down
	if (Keys.q)
		Move.z -= KeyKeyMoveInterval;
	if (Keys.e)
		Move.z += KeyKeyMoveInterval;

	// update camera position
	Position.x += Move.x * cos(Pitch) * cos(Yaw) - Move.y * cos(Pitch) * sin(Yaw);
	Position.y += Move.x * cos(Pitch) * sin(Yaw) + Move.y * cos(Pitch) * cos(Yaw);
	Position.z += Move.z;

	// press down mouse for looking direction
	if (bIsMouseDown)
	{
		Yaw += MouseRotateInterval.x;
		Pitch -= MouseRotateInterval.y;

		// update camera lookat dir
		UpdateLookByEuler(Pitch, Yaw);
	}

	if (Move != FVector(0, 0, 0) || bIsMouseDown)
	{
		IsChanged = true;
	}
}

void ACamera::UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, FVector TargetLocation, float Distance)
{
	const KeysPressed& Keys = FDeviceEventProcessor::Get()->GetKeys();
	const FVector2& MouseMoveDelta = FDeviceEventProcessor::Get()->GetDeltaMouseMove_BottonDown();
	const bool& bIsMouseDown = FDeviceEventProcessor::Get()->IsMouseDown();

	// Interval between tick 
	FVector2 MouseRotateInterval = MouseSensibility * MouseMoveDelta;
	//FVector2 YawPitch = GetEulerByLook(LookDirection);
	//float& Yaw = YawPitch.x;
	//float& Pitch = YawPitch.y;

	FEuler Euler = QuatToEuler(GetStaticMeshCom()->GetTransform().Quat);
	float& Yaw = Euler.Yaw;
	float& Pitch = Euler.Pitch;

	// update lookat dir
	if (bIsMouseDown)
	{
		Yaw += MouseRotateInterval.x;
		Pitch -= MouseRotateInterval.y;
		UpdateLookByEuler(Pitch, Yaw);
	}

	// update position
	FVector HorizontalLook = FVector(GetLook().x, GetLook().y, 0);
	HorizontalLook = glm::normalize(HorizontalLook) * Distance;
	bool IsPosChanged = Position.x != (TargetLocation - HorizontalLook).x || Position.y != (TargetLocation - HorizontalLook).y;
	if (IsPosChanged || bIsMouseDown)
	{
		IsChanged = true;
	}
	Position = TargetLocation - HorizontalLook;
	Position.z = 200.f;
}

void ACamera::UpdateCameraParam_Static(const float& ElapsedSeconds)
{
	// do nothing
	IsChanged = false;
}

FMatrix ACamera::GetViewMatrix() const
{
	return glm::lookAtLH(Position, Position + LookDirection * 10.0f, UpDirection);
}

FMatrix ACamera::GetPerspProjMatrix(const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 1000.0f*/) const
{
	return glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
}

FMatrix ACamera::GetOrthoProjMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 1000.0f*/) const
{
	return glm::orthoLH_ZO(Left, Right, Bottom, Top, NearPlane, FarPlane);
}

void ACamera::Update(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation, float Distance)
{
	switch (Mode)
	{
	case FCameraMoveMode::WANDER:

		UpdateCameraParam_Wander(ElapsedSeconds);
		break;
	case FCameraMoveMode::AROUNDTARGET:

		UpdateCameraParam_AroundTarget(ElapsedSeconds, TargetLocation, Distance);
		break;
	case FCameraMoveMode::STATIC:

		UpdateCameraParam_Static(ElapsedSeconds);
		break;
	default:
		break;
	}
}

FVector2 ACamera::GetEulerByLook(const FVector& LookAt)
{
	// assume rotate from FVector(1, 0, 0)
	float Yaw = Atan2(LookAt.y, LookAt.x);
	float Pitch = Atan2(LookAt.z, sqrt(LookAt.x * LookAt.x + LookAt.y * LookAt.y));
	return FVector2(Yaw, Pitch);
}
