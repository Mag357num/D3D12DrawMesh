#include "Character.h"
#include "SimpleCamera.h"
#include "ActorComponent.h"
#include "SkeletalMesh.h"

void ACharacter::Tick(const float& ElapsedSeconds)
{
	GetSkeletalMeshCom()->TickAnimation(ElapsedSeconds);

	FVector Move(0.f, 0.f, 0.f);
	float MoveInterval = MoveSpeed * ElapsedSeconds;
	float KeyRotateInterval = TurnSpeed * ElapsedSeconds;

	if (Keys.w)
	{
		Move.x += MoveInterval;
	}
	if (Keys.s)
	{
		Move.x -= MoveInterval;
	}
	if (Keys.d)
	{
		Move.y += MoveInterval;
	}
	if (Keys.a)
	{
		Move.y -= MoveInterval;
	}

	FTransform& Trans = GetSkeletalMeshCom()->GetTransform();
	Trans.Translation += Move;
}

void ACharacter::OnKeyDown(const WPARAM& key)
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
	}
}

void ACharacter::OnKeyUp(const WPARAM& key)
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
