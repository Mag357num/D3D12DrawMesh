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

#pragma once

using namespace DirectX;

class FCamera
{
public:
	FCamera();

	void		Init(const XMFLOAT3& position, const XMFLOAT3& upDir, const XMFLOAT3& lookAt);
	void		Update(const float& ElapsedSeconds);
	XMMATRIX	GetViewMatrix();
	XMMATRIX	GetProjectionMatrix(const float& nearPlane = 1.0f, const float& farPlane = 1000.0f);
	void		SetMoveSpeed(const float & UnitsPerSecond);
	void		SetTurnSpeed(const float& RadiansPerSecond);

	void		OnKeyDown(const WPARAM& key);
	void		OnKeyUp(const WPARAM& key);

	void		GetEulerByLook(const XMFLOAT3& lookAt);
	void		GetLookByEuler(const float& pitch, const float& yaw);

	void		SetFov(float FovParam) { Fov = FovParam; }
	void		SetAspectRatio(float AspParam) { AspectRatio = AspParam; }

private:
	void Reset();

	struct KeysPressed
	{
		bool w;
		bool a;
		bool s;
		bool d;
		bool q;
		bool e;

		bool left;
		bool right;
		bool up;
		bool down;
	};

	XMFLOAT3	InitialPosition;
	XMFLOAT3	InitialUpDir;
	XMFLOAT3	InitialLookAt;
	XMFLOAT3	Position;
	XMFLOAT3	LookDirection;
	XMFLOAT3	UpDirection;
	KeysPressed Keys;
	float		MoveSpeed;				// Speed at which the camera moves, in units per second.
	float		TurnSpeed;				// Speed at which the camera turns, in radians per second.
	float		Yaw;						// Relative to the +z axis.
	float		Pitch;					// Relative to the xz plane.

	float Fov = 90.0f;
	float AspectRatio = 1.7777777f;
};
