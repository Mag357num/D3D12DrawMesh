#include "Character.h"

void ACharacter::Tick(const float& ElapsedSeconds)
{
	assert(1); // didnt import skeletalmeshActor 
	SkeletalMeshCom->TickAnimation(ElapsedSeconds);
}
