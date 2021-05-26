#include "StaticMesh.h"

void AStaticMeshActor::SetStaticMeshComponent(shared_ptr<FStaticMeshComponent> Com)
{
	if (Components.size() == 0)
	{
		Components.push_back(Com);
	}
	else
	{
		Components[0] = Com;
	}
}

FStaticMeshComponent* AStaticMeshActor::GetStaticMeshComponent()
{
	if (Components.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return Components[0].get()->As<FStaticMeshComponent>();
	}
}
