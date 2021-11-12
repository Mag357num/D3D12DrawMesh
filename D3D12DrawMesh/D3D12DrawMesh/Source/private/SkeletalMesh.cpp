#include "SkeletalMesh.h"

void ASkeletalMeshActor::SetSkeletalMeshComponent( shared_ptr<FSkeletalMeshComponent> Com )
{
	if (Components.size() == 0)
	{
		Components.push_back( Com );
	}
	else
	{
		Components[0] = Com;
	}
}

FSkeletalMeshComponent* ASkeletalMeshActor::GetSkeletalMeshComponent()
{
	if (Components.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return Components[0].get()->As<FSkeletalMeshComponent>();
	}
}

void FSkeletalMeshComponent::AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq )
{
	if (SkeletalMesh->GetSkeleton() == nullptr)
	{
		throw std::exception( "ERROR: this SkeletalMeshComponent dont have Skeleton" );
	}
	Seq.second->SetSkeleton( SkeletalMesh->GetSkeleton() );
	Animator.AddSequence(Seq);
}
