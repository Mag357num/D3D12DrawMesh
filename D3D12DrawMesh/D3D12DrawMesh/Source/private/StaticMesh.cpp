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

FStaticMeshComponent* AStaticMeshActor::GetStaticMeshCom()
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

void AStaticMeshActor::Tick(const float& ElapsedSeconds)
{
	const FQuat& Quat = GetStaticMeshCom()->GetTransform().Quat;
	auto a = ElapsedSeconds * AngularVelocity;
	GetStaticMeshCom()->SetQuat(glm::rotate(Quat, glm::radians(ElapsedSeconds * AngularVelocity), FVector(0, 0, 1)));

	Dirty = true;
}
