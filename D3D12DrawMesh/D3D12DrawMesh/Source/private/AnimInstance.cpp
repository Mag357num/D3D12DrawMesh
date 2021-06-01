#include "AnimInstance.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"
#include "RenderThread.h"

void FAnimInstance::UpdateAnimation(const float& ElapsedSeconds)
{
	if (TimePos > 100.f)
	{
		TimePos = 0.0f;
	}

	TimePos += ElapsedSeconds;
	float& SequenceLength = SequenceMap[CurrentAnimation]->GetSequenceLength();
	Palette = UpdatePalette(fmod(TimePos, SequenceLength));
	FRenderThread::Get()->UpdatePalette(Palette);
	//test++;
	//FRenderThread::Get()->UpdateTest(test);
}

vector<FMatrix> FAnimInstance::UpdatePalette(float Dt)
{
	vector<FMatrix> Result;
	FSkeleton* Ske = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton();
	vector<FMatrix> JointOffset = Ske->GetJointOffset();

	// root and pelvis use animation translation, other joint use skeleton translation(same with ue4)
	vector<FRetargetType> RetargetList(Ske->GetJoints().size(), FRetargetType::SKELETON);
	RetargetList[0] = FRetargetType::ANIMATION;
	RetargetList[1] = FRetargetType::ANIMATION;

	vector<FMatrix> AnimLocalToParent = SequenceMap[CurrentAnimation]->Interpolate(Dt, Ske, RetargetList);

	vector<FMatrix> AnimGlobalPose;
	AnimGlobalPose.push_back(AnimLocalToParent[0]);

	for (uint32 i = 1; i < AnimLocalToParent.size(); ++i)
	{
		int ParentIndex = SkeletalMeshCom->GetSkeletalMesh()->GetSkeleton()->GetJoints()[i].ParentIndex;
		FMatrix ParentToRoot = AnimGlobalPose[ParentIndex];
		AnimGlobalPose.push_back(ParentToRoot * AnimLocalToParent[i]);
	}

	for (uint32 i = 0; i < AnimGlobalPose.size(); ++i)
	{
		Result.push_back(AnimGlobalPose[i] * JointOffset[i]);
	}

	return Result;
}
