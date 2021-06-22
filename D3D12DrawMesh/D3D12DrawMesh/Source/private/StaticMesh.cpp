#include "StaticMesh.h"

void AStaticMeshActor::Tick(const float& ElapsedSeconds)
{
	const FQuat& Quat = GetStaticMeshComponent()->GetTransform().Quat;
	auto a = ElapsedSeconds * AngularVelocity;
	GetStaticMeshComponent()->SetQuat(glm::rotate(Quat, glm::radians(ElapsedSeconds * AngularVelocity), FVector(0, 0, 1)));

	DirtyCount = true;
}
