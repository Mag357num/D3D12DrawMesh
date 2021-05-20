#include "AnimInstance.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"

void FAnimInstance::initAnimation(FSkeletalMeshComponent* SkeletalMeshCom, shared_ptr<FAnimSequence> Sequence)
{
	Proxy.SkeletalMeshCom = SkeletalMeshCom;
	Proxy.Sequence = Sequence;
}

void FAnimInstance::UpdateAnimation(const float& TotalSeconds)
{
	Proxy.UpdateAnimation(TotalSeconds);
}

void FAnimInstanceProxy::UpdateAnimation(const float& TotalSeconds)
{
	// calculate pose according to tick
	float dt = TotalSeconds - floor(TotalSeconds / Sequence->GetSequenceLength()) * Sequence->GetSequenceLength();
	Palette = UpdatePalette(dt);
}

vector<FMatrix> FAnimInstanceProxy::UpdatePalette(float dt)
{
	vector<FMatrix> Result;
	vector<FMatrix> JointOffset = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJointOffset();
	vector<FMatrix> AnimLocalToParent = Sequence->Interpolate(dt);

	vector<FMatrix> AnimGlobalPose;
	AnimGlobalPose.push_back(AnimLocalToParent[0]);

	for (uint32 i = 1; i < AnimLocalToParent.size(); ++i)
	{
		int ParentIndex = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJoints()[i].ParentIndex;
		FMatrix ParentToRoot = AnimGlobalPose[ParentIndex];
		assert(ParentIndex < i);
		AnimGlobalPose.push_back(ParentToRoot * AnimLocalToParent[i]);
	}

	for (uint32 i = 0; i < AnimGlobalPose.size(); ++i)
	{
		Result.push_back(AnimGlobalPose[i] * JointOffset[i]);
	}

	return Result;
}
