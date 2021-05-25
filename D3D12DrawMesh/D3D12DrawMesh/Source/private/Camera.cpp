#include "Camera.h"
#include <cmath>
#include "MathExtend.h"
#include <gtc/matrix_transform.hpp>

ACamera::ACamera():
	InitialPosition(500.f, 0.f, 0.f),
	Position(InitialPosition),
	LookDirection(-1.f, 0.f, 0.f),
	UpDirection(0.f, 0.f, 1.f),
	MoveSpeed(300.0f),
	MouseSensibility(0.005f),
	Fov(90.f),
	AspectRatio(1.7777777f),
	TurnSpeed(1.570796327f),
	Keys{}
{
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

void ACamera::Init(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float Width, float Height)
{
	InitialPosition = PositionParam;
	InitialUpDir = UpDir;
	InitialLookAt = LookAt;

	Position = PositionParam;
	UpDirection = UpDir;
	LookDirection = LookAt;

	SetFov(Fov);
	SetAspectRatio(Width / Height);

	MouseMove_FirstPosition = FVector2(Width / 2, Height / 2);
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
	FVector Move(0.f, 0.f, 0.f);
	float MoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;
	FVector2 MouseRotateInterval = MouseSensibility * (MouseDown_CurrentPosition - MouseDown_FirstPosition);
	MouseDown_FirstPosition = MouseDown_CurrentPosition;

	FVector2 YawPitch = GetEulerByLook(LookDirection);
	float& Yaw = YawPitch.x;
	float& Pitch = YawPitch.y;

	// wasd for movement
	if (Keys.w)
	{
		Move.x += MoveInterval * cos(Pitch);
		Move.z += MoveInterval * sin(Pitch);
	}
	if (Keys.s)
	{
		Move.x -= MoveInterval * cos(Pitch);
		Move.z -= MoveInterval * sin(Pitch);
	}
	if (Keys.d)
	{
		Move.y += MoveInterval;
	}
	if (Keys.a)
	{
		Move.y -= MoveInterval;
	}

	// up down left right for movement
	if (Keys.up)
	{
		Move.x += MoveInterval * cos(Pitch);
		Move.z += MoveInterval * sin(Pitch);
	}
	if (Keys.down)
	{
		Move.x -= MoveInterval * cos(Pitch);
		Move.z -= MoveInterval * sin(Pitch);
	}
	if (Keys.right)
	{
		Move.y += MoveInterval;
	}
	if (Keys.left)
	{
		Move.y -= MoveInterval;
	}

	// qe for move up and down
	if (Keys.q)
		Move.z -= MoveInterval;
	if (Keys.e)
		Move.z += MoveInterval;

	// update camera position
	Position.x += Move.x * cos(Pitch) * cos(Yaw) - Move.y * cos(Pitch) * sin(Yaw);
	Position.y += Move.x * cos(Pitch) * sin(Yaw) + Move.y * cos(Pitch) * cos(Yaw);
	Position.z += Move.z;

	// press down mouse for looking direction
	if (IsMouseDown)
	{
		Yaw += MouseRotateInterval.x;
		Pitch -= MouseRotateInterval.y;

		// update camera lookat dir
		UpdateLookByEuler(Pitch, Yaw);
	}

	if (Move != FVector(0, 0, 0) || IsMouseDown)
	{
		IsChanged = true;
	}
}

void ACamera::UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, FVector TargetLocation, float Distance)
{
	FVector2 MouseRotateInterval = MouseSensibility * (MouseDown_CurrentPosition - MouseDown_FirstPosition);
	MouseDown_FirstPosition = MouseDown_CurrentPosition;
	FVector2 YawPitch = GetEulerByLook(LookDirection);
	float& Yaw = YawPitch.x;
	float& Pitch = YawPitch.y;

	// update lookat dir
	if (IsMouseDown)
	{
		Yaw += MouseRotateInterval.x;
		Pitch -= MouseRotateInterval.y;
		UpdateLookByEuler(Pitch, Yaw);
	}

	// update position
	FVector HorizontalLook = FVector(GetLook().x, GetLook().y, 0);
	HorizontalLook = glm::normalize(HorizontalLook) * Distance;
	bool IsPosChanged = Position.x != (TargetLocation - HorizontalLook).x || Position.y != (TargetLocation - HorizontalLook).y;
	if (IsPosChanged || IsMouseDown)
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
	return glm::lookAtLH(Position, Position + LookDirection * 10.0f, UpDirection); // TODO: camera didnt update upDir
}

FMatrix ACamera::GetPerspProjMatrix(const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 1000.0f*/) const
{
	return glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
}

FMatrix ACamera::GetOrthoProjMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 1000.0f*/) const
{
	return glm::orthoLH_ZO(Left, Right, Bottom, Top, NearPlane, FarPlane);
}

void ACamera::OnKeyDown(const unsigned char& key)
{
	switch (key)
	{
	case 'W':
		Keys.w = true;
		break;
	case 'A':
		Keys.a = true;
		break;
	case 'S':
		Keys.s = true;
		break;
	case 'D':
		Keys.d = true;
		break;
	case 'Q':
		Keys.q = true;
		break;
	case 'E':
		Keys.e = true;
		break;
	case VK_LEFT:
		Keys.left = true;
		break;
	case VK_RIGHT:
		Keys.right = true;
		break;
	case VK_UP:
		Keys.up = true;
		break;
	case VK_DOWN:
		Keys.down = true;
		break;
	case VK_ESCAPE:
		//Reset();
		break;
	}
}

void ACamera::OnKeyUp(const unsigned char& key)
{
	switch (key)
	{
	case 'W':
		Keys.w = false;
		break;
	case 'A':
		Keys.a = false;
		break;
	case 'S':
		Keys.s = false;
		break;
	case 'D':
		Keys.d = false;
		break;
	case 'Q':
		Keys.q = false;
		break;
	case 'E':
		Keys.e = false;
		break;
	case VK_LEFT:
		Keys.left = false;
		break;
	case VK_RIGHT:
		Keys.right = false;
		break;
	case VK_UP:
		Keys.up = false;
		break;
	case VK_DOWN:
		Keys.down = false;
		break;
	}
}

void ACamera::OnButtonDown(const uint32& x, const uint32& y)
{
	IsMouseDown = true;
	MouseDown_FirstPosition = { static_cast<float>(x), static_cast<float>(y) };
	MouseDown_CurrentPosition = MouseDown_FirstPosition;
}

void ACamera::OnButtonUp()
{
	IsMouseDown = false;
}

void ACamera::OnMouseMove(const uint32& x, const uint32& y)
{
	if (IsMouseDown)
	{
		MouseDown_CurrentPosition = { static_cast<float>(x), static_cast<float>(y) };
	}

	MouseMove_CurrentPosition = { static_cast<float>(x), static_cast<float>(y) };
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
	float Yaw = Atan2(LookAt.y, LookAt.x);
	float Pitch = Atan2(LookAt.z, sqrt(LookAt.x * LookAt.x + LookAt.y * LookAt.y));
	return FVector2(Yaw, Pitch);
}
