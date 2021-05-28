#include "StaticMesh.h"

void TStaticMeshActor::SetStaticMeshComponent(shared_ptr<TStaticMeshComponent> Com)
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

TStaticMeshComponent* TStaticMeshActor::GetStaticMeshComponent()
{
	if (Components.size() == 0)
	{
		return nullptr;
	}
	else
	{
		return Components[0].get()->As<TStaticMeshComponent>();
	}
}
