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

#include "stdafx.h"
#include "SimpleCamera.h"
#include <cmath>
#include "MathExtend.h"
#include <gtc/matrix_transform.hpp>

FCamera::FCamera():
	InitialPosition(500, 0, 0),
	Position(InitialPosition),
	Yaw(XM_PI),
	Pitch(0.0f),
	LookDirection(-1, 0, 0),
	UpDirection(0, 0, 1),
	MoveSpeed(100.0f),
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

	if (fabs(LookDirection.x) < 0.001f) LookDirection.x = 0;
	if (fabs(LookDirection.y) < 0.001f) LookDirection.y = 0;
	if (fabs(LookDirection.z) < 0.001f) LookDirection.z = 0;
}

void FCamera::Init(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt)
{
	InitialPosition = PositionParam;
	InitialUpDir = UpDir;
	InitialLookAt = LookAt;

	Position = PositionParam;
	UpDirection = UpDir;
	LookDirection = LookAt;

	// regard the yaw start at x+ dir, pitch start at x+ dir, roll start at y+ dir.
	GetEulerByLook(LookAt);
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
	FVector move(0, 0, 0);
	float moveInterval = MoveSpeed * ElapsedSeconds;
	float rotateInterval = TurnSpeed * ElapsedSeconds;

	if (Keys.a)
		move.y -= moveInterval;
	if (Keys.d)
		move.y += moveInterval;
	if (Keys.w)
		move.x += moveInterval;
	if (Keys.s)
		move.x -= moveInterval;
	if (Keys.q)
		move.z += moveInterval;
	if (Keys.e)
		move.z -= moveInterval;

	if (Keys.up)
		Pitch += rotateInterval;
	if (Keys.down)
		Pitch -= rotateInterval;
	if (Keys.left)
		Yaw -= rotateInterval;
	if (Keys.right)
		Yaw += rotateInterval;

	Position.x += move.x * cos(Yaw) - move.y * sin(Yaw);
	Position.y += move.x * sin(Yaw) + move.y * cos(Yaw);
	Position.z += move.z;

	GetLookByEuler(Pitch, Yaw);
}

FMatrix FCamera::GetViewMatrix()
{
	return glm::lookAtLH(Position, Position + LookDirection * 10.0f, UpDirection);
}

FMatrix FCamera::GetProjectionMatrix(const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 1000.0f*/)
{
	return glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
}

void FCamera::OnKeyDown(const WPARAM& key)
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
