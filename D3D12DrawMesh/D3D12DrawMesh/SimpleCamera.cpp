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

#include "SimpleCamera.h"
#include <cmath>
#include "MathExtend.h"
#include <gtc/matrix_transform.hpp>

FCamera::FCamera():
	InitialPosition(500.f, 0.f, 0.f),
	Position(InitialPosition),
	Yaw(XM_PI),
	Pitch(0.0f),
	LookDirection(-1.f, 0.f, 0.f),
	UpDirection(0.f, 0.f, 1.f),
	MoveSpeed(300.0f),
	MouseSensibility(0.005f),
	Fov(90.f),
	AspectRatio(1.7777777f),
	TurnSpeed(XM_PIDIV2),
	Keys{}
{
}

void FCamera::GetEulerByLook(const FVector& LookAt)
{
	Yaw = Atan2(LookAt.y, LookAt.x);
	Pitch = Atan2(LookAt.z, sqrt(LookAt.x * LookAt.x + LookAt.y * LookAt.y));
}

void FCamera::GetLookByEuler(const float& Pitch, const float& Yaw)
{
	LookDirection.x = cosf(Pitch) * cosf(Yaw);
	LookDirection.y = cosf(Pitch) * sinf(Yaw);
	LookDirection.z = sinf(Pitch);

	if (fabs(LookDirection.x) < 0.001f) LookDirection.x = 0.f;
	if (fabs(LookDirection.y) < 0.001f) LookDirection.y = 0.f;
	if (fabs(LookDirection.z) < 0.001f) LookDirection.z = 0.f;
}

void FCamera::Init(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float AspectRatio)
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
	SetAspectRatio(AspectRatio);
}

void FCamera::SetMoveSpeed(const float & UnitsPerSecond)
{
	MoveSpeed = UnitsPerSecond;
}

void FCamera::SetTurnSpeed(const float& RadiansPerSecond)
{
	TurnSpeed = RadiansPerSecond;
}

void FCamera::Reset()
{
	Position = InitialPosition;
	UpDirection = InitialUpDir;
	LookDirection = InitialLookAt;
	
	GetEulerByLook(LookDirection);
}

void FCamera::Update(const float& ElapsedSeconds)
{
	FVector Move(0.f, 0.f, 0.f);
	float MoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;

	FVector2 MouseRotateInterval = MouseSensibility * (MouseCurrentPosition - MouseFirstPosition);

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

	
	// qe for move up and down
	if (Keys.q)
		Move.z -= MoveInterval;
	if (Keys.e)
		Move.z += MoveInterval;

	// press down mouse for looking direction
	if (IsMouseDown && IsMouseMove)
		Yaw += MouseRotateInterval.x;
	if (IsMouseDown && IsMouseMove)
		Pitch -= MouseRotateInterval.y;

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

	Position.x += Move.x * cos(Pitch) * cos(Yaw) - Move.y * cos(Pitch) * sin(Yaw);
	Position.y += Move.x * cos(Pitch) * sin(Yaw) + Move.y * cos(Pitch) * cos(Yaw);
	Position.z += Move.z;

	MouseFirstPosition = MouseCurrentPosition;

	GetLookByEuler(Pitch, Yaw);
}

FMatrix FCamera::GetViewMatrix() const
{
	return glm::lookAtLH(Position, Position + LookDirection * 10.0f, UpDirection);
}

FMatrix FCamera::GetProjectionMatrix(const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 1000.0f*/) const
{
	return glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
}

void FCamera::OnKeyDown(const WPARAM& key) //TODO: paltform dependent
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
		Reset();
		break;
	}
}

void FCamera::OnKeyUp(const WPARAM& key)
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

void FCamera::OnRightButtonDown(const uint32& x, const uint32& y)
{
	IsMouseDown = true;
	MouseFirstPosition = { static_cast<float>(x), static_cast<float>(y) };
	MouseCurrentPosition = MouseFirstPosition;
}

void FCamera::OnRightButtonUp()
{
	IsMouseDown = false;
}

void FCamera::OnMouseMove(const uint32& x, const uint32& y)
{
	if (IsMouseDown)
	{
		IsMouseMove = true;
		MouseCurrentPosition = { static_cast<float>(x), static_cast<float>(y) };
	}
}