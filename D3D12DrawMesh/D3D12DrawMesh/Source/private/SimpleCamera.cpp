//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "Camera.h"
#include <cmath>
#include "MathExtend.h"
#include <gtc/matrix_transform.hpp>

ACamera::ACamera():
	InitialPosition(500.f, 0.f, 0.f),
	Position(InitialPosition),
	Yaw(3.141592654f),
	Pitch(0.0f),
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

void ACamera::GetEulerByLook(const FVector& LookAt)
{
	Yaw = Atan2(LookAt.y, LookAt.x);
	Pitch = Atan2(LookAt.z, sqrt(LookAt.x * LookAt.x + LookAt.y * LookAt.y));
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

	// regard the yaw start at x+ dir, pitch start at x+ dir, roll start at y+ dir.
	GetEulerByLook(LookAt);

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
	
	GetEulerByLook(LookDirection);
}

void ACamera::Update_Wander(const float& ElapsedSeconds)
{
	FVector Move(0.f, 0.f, 0.f);
	float MoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;

	FVector2 MouseRotateInterval = MouseSensibility * (MouseDown_CurrentPosition - MouseDown_FirstPosition);

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

	// press down mouse for looking direction
	if (IsMouseDown)
	{
		Yaw += MouseRotateInterval.x;
		Pitch -= MouseRotateInterval.y;
	}

	Position.x += Move.x * cos(Pitch) * cos(Yaw) - Move.y * cos(Pitch) * sin(Yaw);
	Position.y += Move.x * cos(Pitch) * sin(Yaw) + Move.y * cos(Pitch) * cos(Yaw);
	Position.z += Move.z;

	MouseDown_FirstPosition = MouseDown_CurrentPosition;

	UpdateLookByEuler(Pitch, Yaw);
}

void ACamera::Update_AroundTarget(const float& ElapsedSeconds, FVector TargetLocation, float Distance)
{
	FVector2 MouseRotateInterval = MouseSensibility * (MouseDown_CurrentPosition - MouseDown_FirstPosition);
	MouseDown_FirstPosition = MouseDown_CurrentPosition;

	Yaw += MouseRotateInterval.x;
	Pitch -= MouseRotateInterval.y;
	UpdateLookByEuler(Pitch, Yaw);

	FVector horizontalLook = FVector(GetLook().x, GetLook().y, 0);
	horizontalLook = glm::normalize(horizontalLook) * Distance;
	Position = TargetLocation - horizontalLook;
	Position.z = 200.f;
}

void ACamera::Update_Static(const float& ElapsedSeconds)
{
	// do nothing
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

void ACamera::OnKeyDown(const WPARAM& key) //TODO: paltform dependent
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
		//Reset(); // TODO: delete temporary
		break;
	}
}

void ACamera::OnKeyUp(const WPARAM& key)
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

void ACamera::OnRightButtonDown(const uint32& x, const uint32& y)
{
	IsMouseDown = true;
	MouseDown_FirstPosition = { static_cast<float>(x), static_cast<float>(y) };
	MouseDown_CurrentPosition = MouseDown_FirstPosition;
}

void ACamera::OnRightButtonUp()
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
