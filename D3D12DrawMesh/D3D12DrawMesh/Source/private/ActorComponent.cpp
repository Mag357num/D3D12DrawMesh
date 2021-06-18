#include "ActorComponent.h"

void FMeshComponent::SetMaterial(shared_ptr<FMaterialInterface> Mat, uint32 index)
{
	if (Materials.size() < index + 1)
	{
		Materials.resize(index + 1);
	}

	Materials[index] = Mat;
}
