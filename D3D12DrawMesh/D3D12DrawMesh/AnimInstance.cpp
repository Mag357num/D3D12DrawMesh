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
	float dt = TotalSeconds - floor(TotalSeconds / Sequence->GetSequenceLength());
	Palette = GetFinalTransforms(dt);
}

vector<FMatrix> FAnimInstanceProxy::GetFinalTransforms(float dt)
{
	vector<FMatrix> Result;
	vector<FMatrix> JointOffset = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJointOffset();
	vector<FMatrix> LocalPose = Sequence->Interpolate(dt);

	vector<FMatrix> GlobalPose;
	GlobalPose.push_back(LocalPose[0]);

	for (uint32 i = 1; i < LocalPose.size(); ++i)
	{
		int ParentIndex = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJoints()[i].ParentIndex;
		FMatrix ParentToRoot = GlobalPose[ParentIndex];

		GlobalPose.push_back(ParentToRoot * LocalPose[i]);
	}

	for (uint32 i = 0; i < GlobalPose.size(); ++i)
	{
		Result.push_back(GlobalPose[i] * JointOffset[i]);
	}

	return Result;
}
