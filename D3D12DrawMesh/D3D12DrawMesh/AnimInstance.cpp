#include "AnimInstance.h"

void FAnimInstance::initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, FAnimSequenceBase* Sequence)
{
	Proxy.SkeletalMeshCom = SkeletalMeshCom;
	Proxy.Sequence = Sequence;
}

void FAnimInstance::UpdateAnimation()
{
	assert(0);
}

