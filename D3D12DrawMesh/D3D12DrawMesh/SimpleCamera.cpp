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

void FCamera::GetEulerByLook(const XMFLOAT3& lookAt)
{
	Yaw = Atan2(lookAt.y, lookAt.x);
	Pitch = Atan2(lookAt.z, sqrt(lookAt.x * lookAt.x + lookAt.y * lookAt.y));
}

void FCamera::GetLookByEuler(const float& pitch, const float& yaw)
{
	LookDirection.x = cosf(pitch) * cosf(yaw);
	LookDirection.y = cosf(pitch) * sinf(yaw);
	LookDirection.z = sinf(pitch);

	if (fabs(LookDirection.x) < 0.001f) LookDirection.x = 0;
	if (fabs(LookDirection.y) < 0.001f) LookDirection.y = 0;
	if (fabs(LookDirection.z) < 0.001f) LookDirection.z = 0;
}

void FCamera::Init(const XMFLOAT3& position, const XMFLOAT3& upDir, const XMFLOAT3& lookAt)
{
	InitialPosition = position;
	InitialUpDir = upDir;
	InitialLookAt = lookAt;

	Position = position;
	UpDirection = upDir;
	LookDirection = lookAt;

	// regard the yaw start at x+ dir, pitch start at x+ dir, roll start at y+ dir.
	GetEulerByLook(lookAt);
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
	XMFLOAT3 move(0, 0, 0);
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

XMMATRIX FCamera::GetViewMatrix()
{
	return XMMatrixLookToLH(XMLoadFloat3(&Position), XMLoadFloat3(&LookDirection), XMLoadFloat3(&UpDirection));
}

XMMATRIX FCamera::GetProjectionMatrix(const float& fov, const float& aspectRatio, const float& nearPlane, const float& farPlane)
{
	return XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
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
