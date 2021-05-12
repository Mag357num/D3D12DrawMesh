#include "Character.h"

void ACharacter::Tick(const float& ElapsedSeconds)
{
	assert(0); // didnt import skeletalmeshActor
	SkeletalMeshActor->GetComs()[0]->As<FSkeletalMeshComponent>()->TickAnimation(ElapsedSeconds);
}
