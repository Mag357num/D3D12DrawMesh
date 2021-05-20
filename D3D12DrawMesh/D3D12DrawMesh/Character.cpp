#include "Character.h"

void ACharacter::Tick(const float& TotalSeconds)
{
	GetSkeletalMeshCom()->TickAnimation(TotalSeconds);
}
