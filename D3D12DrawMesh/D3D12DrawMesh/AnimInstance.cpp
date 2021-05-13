#include "AnimInstance.h"

void FAnimInstance::initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, FAnimSequenceBase* Sequence)
{
	Proxy.SkeletalMeshCom = SkeletalMeshCom;
	Proxy.Sequence = Sequence;
}

void FAnimInstance::UpdateAnimation(const float& ElapsedSeconds)
{
	Proxy.UpdateAnimation(ElapsedSeconds);
}

void FAnimInstanceProxy::UpdateAnimation(const float& ElapsedSeconds)
{
	// calculate pose according to tick

}
