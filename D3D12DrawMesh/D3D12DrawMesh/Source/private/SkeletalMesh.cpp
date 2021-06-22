#include "SkeletalMesh.h"
#include "DeviceEventProcessor.h"

void FSkeletalMeshComponent::AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq )
{
	if (SkeletalMesh->GetSkeleton() == nullptr)
	{
		throw std::exception( "ERROR: this SkeletalMeshComponent dont have Skeleton" );
	}
	Seq.second->SetSkeleton( SkeletalMesh->GetSkeleton() );
	Animator.AddSequence(Seq);
}

void ASkeletalMeshActor::Tick(const float& ElapsedSeconds)
{
	GetSkeletalMeshComponent()->TickAnimation(ElapsedSeconds);
}

void ASkeletalMeshActor::SetCurrentAnim(string Key)
{
	GetSkeletalMeshComponent()->GetAnimator().SetCurrentAnim(Key);
}
