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

class ACamera // TODO: refactor: make ACamera a Actor
{
public:
	ACamera();

	void Init(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float Width, float Height);
	void Update_Wander(const float& ElapsedSeconds);
	void Update_AroundTarget(const float& ElapsedSeconds, FVector TargetLocation, float Distance);
	void Update_Static(const float& ElapsedSeconds);
	FMatrix GetViewMatrix() const;
	FMatrix GetPerspProjMatrix(const float& NearPlane = 1.0f, const float& FarPlane = 1000.0f) const;
	FMatrix GetOrthoProjMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top ,const float& NearPlane = 1.0f, const float& FarPlane = 1000.0f) const;
	void SetMoveSpeed(const float & UnitsPerSecond);
	void SetTurnSpeed(const float& RadiansPerSecond);

	void OnKeyDown(const WPARAM& key);
	void OnKeyUp(const WPARAM& key);

	void OnRightButtonDown(const uint32& x, const uint32& y);
	void OnRightButtonUp();
	void OnMouseMove(const uint32& x, const uint32& y);

	void GetEulerByLook(const FVector& LookAt);
	void UpdateLookByEuler(const float& Pitch, const float& Yaw);

	void SetFov(const float& FovParam) { Fov = FovParam; }
	void SetAspectRatio(const float& AspParam) { AspectRatio = AspParam; }

	FVector GetPosition() { return Position; }
	FVector GetLook() { return LookDirection; }

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

	FVector InitialPosition;
	FVector InitialUpDir;
	FVector InitialLookAt;

	FVector Position;
	FVector LookDirection;
	FVector UpDirection;

	KeysPressed Keys;

	FVector2 MouseDown_CurrentPosition;
	FVector2 MouseDown_FirstPosition;
	FVector2 MouseMove_FirstPosition;
	FVector2 MouseMove_CurrentPosition;

	bool IsMouseDown;

	float MoveSpeed; // Speed at which the camera moves, in units per second.
	float TurnSpeed; // Speed at which the camera turns, in radians per second.

	float MouseSensibility;

	float Yaw; // Relative to the +z axis. // TODO: use quaternion to replace euler angle and look dir
	float Pitch; // Relative to the xz plane.

	float Fov;
	float AspectRatio;
};
