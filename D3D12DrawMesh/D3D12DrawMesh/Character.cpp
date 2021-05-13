#include "Character.h"

void ACharacter::Tick(const float& ElapsedSeconds)
{
	assert(1); // didnt import skeletalmeshActor 
	SkeletalMeshCom->GetComs()[0]->As<FSkeletalMeshComponent>()->TickAnimation(ElapsedSeconds);
}
