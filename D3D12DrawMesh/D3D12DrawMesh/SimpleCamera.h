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

	void Init(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float AspectRatio);
	void		Update(const float& ElapsedSeconds);
	FMatrix		GetViewMatrix() const;
	FMatrix		GetProjectionMatrix(const float& NearPlane = 1.0f, const float& FarPlane = 1000.0f) const;
	void		SetMoveSpeed(const float & UnitsPerSecond);
	void		SetTurnSpeed(const float& RadiansPerSecond);

	void		OnKeyDown(const WPARAM& key);
	void		OnKeyUp(const WPARAM& key);

	void		GetEulerByLook(const FVector& LookAt);
	void		GetLookByEuler(const float& Pitch, const float& Yaw);

	void		SetFov(const float& FovParam) { Fov = FovParam; }
	void		SetAspectRatio(const float& AspParam) { AspectRatio = AspParam; }

	FVector		GetPosition() { return Position; }

private:
	void		Reset();

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

	FVector		InitialPosition;
	FVector		InitialUpDir;
	FVector		InitialLookAt;
	FVector		Position;
	FVector		LookDirection;
	FVector		UpDirection;
	KeysPressed	Keys;
	float		MoveSpeed;				// Speed at which the camera moves, in units per second.
	float		TurnSpeed;				// Speed at which the camera turns, in radians per second.
	float		Yaw;						// Relative to the +z axis.
	float		Pitch;					// Relative to the xz plane.

	float		Fov = 90.0f;
	float		AspectRatio = 1.7777777f;
};
