#include "Character.h"

void ACharacter::Tick(const float& TotalSeconds)
{
	assert(1); // didnt import skeletalmeshActor 
	GetSkeletalMeshCom()->TickAnimation(TotalSeconds);
}
