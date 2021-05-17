#include "AnimInstance.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"

void FAnimInstance::initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, shared_ptr<FAnimSequence> Sequence)
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

FMatrix FAnimInstanceProxy::GetFinalTransforms(float dt)
{
	FMatrix Result;

	vector<FMatrix> JointOffset = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJointOffset();

	vector<FMatrix> toParentTransforms(JointOffset.size());

	return Result;
}
