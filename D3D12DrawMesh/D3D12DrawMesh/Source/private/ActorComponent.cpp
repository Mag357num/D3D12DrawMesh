#include "ActorComponent.h"
#include "Actor.h"

void FMeshComponent::SetMaterial(shared_ptr<FMaterialInterface> Mat, uint32 index)
{
	if (Materials.size() < index + 1)
	{
		Materials.resize(index + 1);
	}

	Materials[index] = Mat;
}

void FSceneComponent::AttachToComponent(FSceneComponent* InParent)
{
	AttachParent = InParent;
	InParent->AttachChildren.push_back(this);
	InParent->GetOwner()->AddOwnedComponent(dynamic_pointer_cast<FSceneComponent>(shared_from_this()));
}

void FSceneComponent::DetachFromComponent()
{
	for (auto i = AttachParent->AttachChildren.begin(); i != AttachParent->AttachChildren.end(); i++)
	{
		if (*i == this)
		{
			AttachParent->AttachChildren.erase(i);
			break;
		}
	}
	AttachParent = nullptr;

	vector<shared_ptr<FSceneComponent>>& OwnerOwnedComponents = GetOwner()->GetOwnedComponents();
	for (auto i = OwnerOwnedComponents.begin(); i != OwnerOwnedComponents.end(); i++)
	{
		if ((*i).get() == this)
		{
			OwnerOwnedComponents.erase(i);
			break;
		}
	}
	SetOwner(nullptr);
}
