#include "Character.h"
#include "SkeletalMesh.h"
#include "DeviceEventProcessor.h"

void ACharacter::Tick(const float& ElapsedSeconds)
{
	GetSkeletalMeshCom()->TickAnimation(ElapsedSeconds);
	TickPosition(ElapsedSeconds);
}

void ACharacter::TickPosition(const float& ElapsedSeconds)
{
	const KeysPressed& Keys = FDeviceEventProcessor::Get()->GetKeys();
	const FVector2& MouseMoveDelta = FDeviceEventProcessor::Get()->GetDeltaMouseMove_BottonDown();

	if (Keys.IsEmpty() || FDeviceEventProcessor::Get()->IsMouseDown())
	{
		MarkDirty();
	}

	// Interval between tick 
	float KeyMoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;
	FVector2 MouseRotateInterval = MouseSensibility * MouseMoveDelta;

	FVector Move(0.f, 0.f, 0.f);
	if (Keys.w)
	{
		Move.y += KeyMoveInterval; // push w go forward because the character model face (0,1,0)
	}
	if (Keys.s)
	{
		Move.y -= KeyMoveInterval;
	}
	if (Keys.d)
	{
		Move.x -= KeyMoveInterval;
	}
	if (Keys.a)
	{
		Move.x += KeyMoveInterval;
	}

	FEuler Euler = QuatToEuler(GetSkeletalMeshCom()->GetTransform().Quat); // roll pitch yaw
	Euler.Yaw += MouseRotateInterval.x; // yaw

	FTransform Trans = GetSkeletalMeshCom()->GetTransform();
	Trans.Translation.x += Move.x * cos(Euler.Yaw) - Move.y * sin(Euler.Yaw);
	Trans.Translation.y += Move.x * sin(Euler.Yaw) + Move.y * cos(Euler.Yaw);
	Trans.Quat = EulerToQuat(Euler);
	GetSkeletalMeshCom()->SetTransform(Trans);
}

void ACharacter::SetCurrentAnim(string Key)
{
	GetSkeletalMeshCom()->GetAnimator().SetCurrentAnim( Key );
}
