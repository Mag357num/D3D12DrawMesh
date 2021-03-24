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

SimpleCamera::SimpleCamera():
	m_initialPosition(500, 0, 0),
	m_position(m_initialPosition),
	m_yaw(XM_PI),
	m_pitch(0.0f),
	m_lookDirection(-1, 0, 0),
	m_upDirection(0, 0, 1),
	m_moveSpeed(100.0f),
	m_turnSpeed(XM_PIDIV2),
	m_keysPressed{}
{
}

float SimpleCamera::Atan2(const float& Y, const float& X)
{
	const float absX = fabs(X);
	const float absY = fabs(Y);
	const bool yAbsBigger = (absY > absX);
	float t0 = yAbsBigger ? absY : absX; // Max(absY, absX)
	float t1 = yAbsBigger ? absX : absY; // Min(absX, absY)

	if (t0 == 0.f)
		return 0.f;

	float t3 = t1 / t0;
	float t4 = t3 * t3;

	static const float c[7] = {
		+7.2128853633444123e-03f,
		-3.5059680836411644e-02f,
		+8.1675882859940430e-02f,
		-1.3374657325451267e-01f,
		+1.9856563505717162e-01f,
		-3.3324998579202170e-01f,
		+1.0f
	};

	t0 = c[0];
	t0 = t0 * t4 + c[1];
	t0 = t0 * t4 + c[2];
	t0 = t0 * t4 + c[3];
	t0 = t0 * t4 + c[4];
	t0 = t0 * t4 + c[5];
	t0 = t0 * t4 + c[6];
	t3 = t0 * t3;

	t3 = yAbsBigger ? (0.5f * XM_PI) - t3 : t3;
	t3 = (X < 0.0f) ? XM_PI - t3 : t3;
	t3 = (Y < 0.0f) ? -t3 : t3;

	return t3;
}

void SimpleCamera::GetEulerByLook(const XMFLOAT3& lookAt)
{
	// Find yaw.
	m_yaw = Atan2(lookAt.y, lookAt.x);

	// Find pitch.
	m_pitch = Atan2(lookAt.z, sqrt(lookAt.x * lookAt.x + lookAt.y * lookAt.y));
}

void SimpleCamera::GetLookByEuler(const float& pitch, const float& yaw)
{
	m_lookDirection.x = cosf(pitch) * cosf(yaw);
	m_lookDirection.y = cosf(pitch) * sinf(yaw);
	m_lookDirection.z = sinf(pitch);

	if (fabs(m_lookDirection.x) < 0.001f) m_lookDirection.x = 0;
	if (fabs(m_lookDirection.y) < 0.001f) m_lookDirection.y = 0;
	if (fabs(m_lookDirection.z) < 0.001f) m_lookDirection.z = 0;
}

void SimpleCamera::Init(XMFLOAT3 position, XMFLOAT3 upDir, XMFLOAT3 lookAt)
{
	m_initialPosition = position;
	m_initialUpDir = upDir;
	m_initialLookAt = lookAt;

	m_position = position;
	m_upDirection = upDir;
	m_lookDirection = lookAt;

	// regard the yaw start at x+ dir, pitch start at x+ dir, roll start at y+ dir.
	GetEulerByLook(lookAt);
}

void SimpleCamera::SetMoveSpeed(float unitsPerSecond)
{
	m_moveSpeed = unitsPerSecond;
}

void SimpleCamera::SetTurnSpeed(float radiansPerSecond)
{
	m_turnSpeed = radiansPerSecond;
}

void SimpleCamera::Reset()
{
	m_position = m_initialPosition;
	m_upDirection = m_initialUpDir;
	m_lookDirection = m_initialLookAt;
	
	GetEulerByLook(m_lookDirection);
}

void SimpleCamera::Update(float elapsedSeconds)
{
	XMFLOAT3 move(0, 0, 0);
	float moveInterval = m_moveSpeed * elapsedSeconds;
	float rotateInterval = m_turnSpeed * elapsedSeconds;

	if (m_keysPressed.a)
		move.y -= moveInterval;
	if (m_keysPressed.d)
		move.y += moveInterval;
	if (m_keysPressed.w)
		move.x += moveInterval;
	if (m_keysPressed.s)
		move.x -= moveInterval;
	if (m_keysPressed.q)
		move.z += moveInterval;
	if (m_keysPressed.e)
		move.z -= moveInterval;

	if (m_keysPressed.up)
		m_pitch += rotateInterval;
	if (m_keysPressed.down)
		m_pitch -= rotateInterval;
	if (m_keysPressed.left)
		m_yaw -= rotateInterval;
	if (m_keysPressed.right)
		m_yaw += rotateInterval;

	m_position.x += move.x * cos(m_yaw) - move.y * sin(m_yaw);
	m_position.y += move.x * sin(m_yaw) + move.y * cos(m_yaw);
	m_position.z += move.z;

	GetLookByEuler(m_pitch, m_yaw);
}

XMMATRIX SimpleCamera::GetViewMatrix()
{
	return XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookDirection), XMLoadFloat3(&m_upDirection));
}

XMMATRIX SimpleCamera::GetProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
}

void SimpleCamera::OnKeyDown(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keysPressed.w = true;
		break;
	case 'A':
		m_keysPressed.a = true;
		break;
	case 'S':
		m_keysPressed.s = true;
		break;
	case 'D':
		m_keysPressed.d = true;
		break;
	case 'Q':
		m_keysPressed.q = true;
		break;
	case 'E':
		m_keysPressed.e = true;
		break;
	case VK_LEFT:
		m_keysPressed.left = true;
		break;
	case VK_RIGHT:
		m_keysPressed.right = true;
		break;
	case VK_UP:
		m_keysPressed.up = true;
		break;
	case VK_DOWN:
		m_keysPressed.down = true;
		break;
	case VK_ESCAPE:
		Reset();
		break;
	}
}

void SimpleCamera::OnKeyUp(WPARAM key)
{
	switch (key)
	{
	case 'W':
		m_keysPressed.w = false;
		break;
	case 'A':
		m_keysPressed.a = false;
		break;
	case 'S':
		m_keysPressed.s = false;
		break;
	case 'D':
		m_keysPressed.d = false;
		break;
	case 'Q':
		m_keysPressed.q = false;
		break;
	case 'E':
		m_keysPressed.e = false;
		break;
	case VK_LEFT:
		m_keysPressed.left = false;
		break;
	case VK_RIGHT:
		m_keysPressed.right = false;
		break;
	case VK_UP:
		m_keysPressed.up = false;
		break;
	case VK_DOWN:
		m_keysPressed.down = false;
		break;
	}
}
