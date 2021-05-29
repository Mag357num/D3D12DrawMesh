#include "SkeletalMesh.h"

void ASkeletalMeshActor::SetSkeletalMeshComponent( shared_ptr<TSkeletalMeshComponent> Com )
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

TSkeletalMeshComponent* ASkeletalMeshActor::GetSkeletalMeshComponent()
{
	if (Components.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return Components[0].get()->As<TSkeletalMeshComponent>();
	}
}

void TSkeletalMeshComponent::AddSequence( std::pair<string, shared_ptr<FAnimSequence>> Seq )
{
	if (SkeletalMesh->GetSkeleton() == nullptr)
	{
		throw std::exception( "ERROR: this SkeletalMeshComponent dont have Skeleton" );
	}
	Seq.second->SetSkeleton( SkeletalMesh->GetSkeleton() );
	Animator.AddSequence(Seq);
}
