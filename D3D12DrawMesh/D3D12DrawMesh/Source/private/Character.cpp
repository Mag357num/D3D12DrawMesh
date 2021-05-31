#include "Character.h"
#include "Camera.h"
#include "ActorComponent.h"
#include "SkeletalMesh.h"

void ACharacter::Tick(const float& ElapsedSeconds)
{
	GetSkeletalMeshCom()->TickAnimation(ElapsedSeconds);

	FVector Move(0.f, 0.f, 0.f);
	float MoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;
	FVector2 MouseRotateInterval = MouseSensibility * (MouseDown_CurrentPosition - MouseDown_FirstPosition);

	if (Keys.w)
	{
		Move.y += MoveInterval; // push w go forward because the character model face (0,1,0)
	}
	if (Keys.s)
	{
		Move.y -= MoveInterval;
	}
	if (Keys.d)
	{
		Move.x -= MoveInterval;
	}
	if (Keys.a)
	{
		Move.x += MoveInterval;
	}

	FEuler Euler = QuatToEuler(GetSkeletalMeshCom()->GetTransform().Quat); // roll pitch yaw
	Euler.Yaw += MouseRotateInterval.x; // yaw
	MouseDown_FirstPosition = MouseDown_CurrentPosition;

	FTransform Trans = GetSkeletalMeshCom()->GetTransform();
	Trans.Translation.x += Move.x * cos(Euler.Yaw) - Move.y * sin(Euler.Yaw);
	Trans.Translation.y += Move.x * sin(Euler.Yaw) + Move.y * cos(Euler.Yaw);
	Trans.Quat = EulerToQuat(Euler);
}

void ACharacter::OnKeyDown(const unsigned char& key)
{
	SetCurrentAnim( "Run" );

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
	}
}

void ACharacter::OnKeyUp(const unsigned char& key)
{
	SetCurrentAnim( "Idle" );

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

void ACharacter::OnButtonDown(const uint32& x, const uint32& y)
{
	IsMouseDown = true;
	MouseDown_FirstPosition = { static_cast<float>(x), static_cast<float>(y) };
	MouseDown_CurrentPosition = MouseDown_FirstPosition;
}

void ACharacter::OnButtonUp()
{
	IsMouseDown = false;
}

void ACharacter::OnMouseMove(const uint32& x, const uint32& y)
{
	if (IsMouseDown)
	{
		MouseDown_CurrentPosition = { static_cast<float>(x), static_cast<float>(y) };
	}

	MouseMove_CurrentPosition = { static_cast<float>(x), static_cast<float>(y) };
}

void ACharacter::SetCurrentAnim(string Key)
{
	GetSkeletalMeshCom()->GetAnimator().SetCurrentAnim( Key );
}
