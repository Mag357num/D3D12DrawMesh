#include "Camera.h"
#include "StaticMesh.h"
#include "DeviceEventProcessor.h"
#include "RenderThread.h"

ACamera::ACamera()
{
	Components.push_back(make_shared<FStaticMeshComponent>());
}

ACamera::ACamera(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane/* = 1.0f*/, const float& FarPlane/* = 5000.0f*/)
{
	Components.push_back(make_shared<FStaticMeshComponent>());
	Init(Eye, Up, LookAt, Fov, Width, Height, NearPlane, FarPlane );
}

void ACamera::Init(const FVector& Eye, const FVector& Up, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane, const float& FarPlane)
{
	InitialPosition = Eye;
	InitialUpDir = Up;
	InitialLookAt = LookAt;

	// eye, look, up construct a view matrix and view matrix is inverse matrix of camera entity
	VMatrix = glm::lookAtLH(Eye, Eye + LookAt * 10.0f, Up);
	VDirty = false;
	SetWorldMatrix(glm::inverse(VMatrix));

	SetFov(Fov);
	SetAspectRatio(Width / Height);
	SetViewPlane(NearPlane, FarPlane);
	PMatrix = glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
	PDirty = false;
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
}

void ACamera::UpdateCameraParam_Wander(const float& ElapsedSeconds)
{
	//const KeysPressed& Keys = FDeviceEventProcessor::Get()->GetKeys();
	//const FVector2& MouseMoveDelta = FDeviceEventProcessor::Get()->GetDeltaMouseMove_BottonDown();
	//const bool& bIsMouseDown = FDeviceEventProcessor::Get()->IsMouseDown();

	//// Interval between tick 
	//float KeyKeyMoveInterval = MoveSpeed * ElapsedSeconds;
	//float KeyRotateInterval = TurnSpeed * ElapsedSeconds;
	//FVector2 MouseRotateInterval = MouseSensibility * MouseMoveDelta;

	//FVector2 YawPitch = GetEulerByLook(LookDirection);
	//float& Yaw = YawPitch.x;
	//float& Pitch = YawPitch.y;

	//// wasd for movement
	//FVector Move(0.f, 0.f, 0.f);
	//if (Keys.w)
	//{
	//	Move.x += KeyKeyMoveInterval * cos(Pitch);
	//	Move.z += KeyKeyMoveInterval * sin(Pitch);
	//}
	//if (Keys.s)
	//{
	//	Move.x -= KeyKeyMoveInterval * cos(Pitch);
	//	Move.z -= KeyKeyMoveInterval * sin(Pitch);
	//}
	//if (Keys.d)
	//{
	//	Move.y += KeyKeyMoveInterval;
	//}
	//if (Keys.a)
	//{
	//	Move.y -= KeyKeyMoveInterval;
	//}

	//// up down left right for movement
	//if (Keys.up)
	//{
	//	Move.x += KeyKeyMoveInterval * cos(Pitch);
	//	Move.z += KeyKeyMoveInterval * sin(Pitch);
	//}
	//if (Keys.down)
	//{
	//	Move.x -= KeyKeyMoveInterval * cos(Pitch);
	//	Move.z -= KeyKeyMoveInterval * sin(Pitch);
	//}
	//if (Keys.right)
	//{
	//	Move.y += KeyKeyMoveInterval;
	//}
	//if (Keys.left)
	//{
	//	Move.y -= KeyKeyMoveInterval;
	//}

	//// qe for move up and down
	//if (Keys.q)
	//	Move.z -= KeyKeyMoveInterval;
	//if (Keys.e)
	//	Move.z += KeyKeyMoveInterval;

	//// update camera position
	//Position.x += Move.x * cos(Pitch) * cos(Yaw) - Move.y * cos(Pitch) * sin(Yaw);
	//Position.y += Move.x * cos(Pitch) * sin(Yaw) + Move.y * cos(Pitch) * cos(Yaw);
	//Position.z += Move.z;

	//// press down mouse for looking direction
	//if (bIsMouseDown)
	//{
	//	Yaw += MouseRotateInterval.x;
	//	Pitch -= MouseRotateInterval.y;

	//	// update camera lookat dir
	//	UpdateLookByEuler(Pitch, Yaw);
	//}

	//if (Move != FVector(0, 0, 0) || bIsMouseDown)
	//{
	//	IsChanged = true;
	//}
}

void ACamera::UpdateCameraParam_AroundTarget(const float& ElapsedSeconds, const FVector& TargetPos, const float& Distance)
{
	// event data
	const KeysPressed& Keys = FDeviceEventProcessor::Get()->GetKeys();
	const FVector2& MouseMoveDelta = FDeviceEventProcessor::Get()->GetDeltaMouseMove_BottonDown();
	const bool& bIsMouseDown = FDeviceEventProcessor::Get()->IsMouseDown();
	FVector2 MouseRotateInterval = MouseSensibility * MouseMoveDelta; // Interval between tick

	FEuler Euler = QuatToEuler(GetTransform().Quat);
	float& Yaw = Euler.Yaw;
	float& Pitch = Euler.Pitch;
	float& Roll = Euler.Roll;

	// update lookat dir
	if (bIsMouseDown)
	{
		Yaw += MouseRotateInterval.x;
		Roll += MouseRotateInterval.y;
		SetQuat(EulerToQuat(FEuler(Roll, Pitch, Yaw)));
	}

	// update position
	const FVector TargetPosLift = TargetPos + FVector( 0, 0, 200 ); // lift the camera for 200
	const FVector LookDir = GetWorldMatrix() * FVector4(0, 0, 1, 0); // directional vector dont translate
	FVector HorizontalLook = glm::normalize(FVector( LookDir.x, LookDir.y, 0 )) * Distance;
	const FVector TheoryPos = TargetPosLift - HorizontalLook;
	const FVector& ActualPos = GetTransform().Translation;
	if (ActualPos != TheoryPos) // change position or rotate view direction
	{
		SetTranslate(TheoryPos);
	}

	if (bIsMouseDown || ActualPos != TheoryPos)
	{
		FRenderThread::Get()->WaitForRenderThread();
		FRenderThread::Get()->UpdateFrameResCamera(transpose(GetPerspProjMatrix() * GetViewMatrix()), GetTransform().Translation);
	}
}

void ACamera::UpdateCameraParam_Static(const float& ElapsedSeconds)
{
	// do nothing
}

FMatrix ACamera::GetViewMatrix()
{
	if (VDirty)
	{
		VMatrix = inverse(Components[0]->GetWorldMatrix());
		VDirty = false;
	}
	return VMatrix;
}

FMatrix ACamera::GetPerspProjMatrix()
{
	if (PDirty)
	{
		PMatrix = glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
		PDirty = false;
	}
	return PMatrix;
}

FMatrix ACamera::GetOrthoProjMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& NearPlane /*= 1.0f*/, const float& FarPlane /*= 5000.0f*/) const
{
	return glm::orthoLH_ZO(Left, Right, Bottom, Top, NearPlane, FarPlane);
}

void ACamera::SetQuat(const FQuat& Quat)
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Camera dont have component!");
	}
	else
	{
		Components[0]->SetQuat(Quat);
		VDirty = true;
	}
}

void ACamera::SetTranslate(const FVector& Trans)
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Camera dont have component!");
	}
	else
	{
		Components[0]->SetTranslate(Trans);
		VDirty = true;
	}
}

void ACamera::SetWorldMatrix(const FMatrix& W)
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Camera dont have component!");
	}
	else
	{
		Components[0]->SetWorldMatrix(W);
		VDirty = true;
	}
}

const FTransform& ACamera::GetTransform()
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Camera dont have component!");
	}
	else
	{
		return Components[0]->GetTransform();
	}
}

const FMatrix& ACamera::GetWorldMatrix()
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Camera dont have component!");
	}
	else
	{
		return Components[0]->GetWorldMatrix();
	}
}

void ACamera::Tick(const float& ElapsedSeconds, FCameraMoveMode Mode, FVector TargetLocation, float Distance)
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