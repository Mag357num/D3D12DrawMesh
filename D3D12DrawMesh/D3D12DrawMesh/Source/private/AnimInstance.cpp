#include "AnimInstance.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"

void FAnimInstance::UpdateAnimation(const float& ElapsedSeconds)
{
	if (TimePos > 100.f)
	{
		TimePos = 0.0f;
	}

	TimePos += ElapsedSeconds;
	float dt = TimePos - floor(TimePos / SequenceMap[CurrentAnimation]->GetSequenceLength() ) * SequenceMap[CurrentAnimation]->GetSequenceLength();
	Palette = UpdatePalette(dt);
}

vector<FMatrix> FAnimInstance::UpdatePalette(float dt)
{
	vector<FMatrix> Result;
	vector<FMatrix> JointOffset = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJointOffset();
	vector<FMatrix> AnimLocalToParent = SequenceMap[CurrentAnimation]->Interpolate(dt);

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
