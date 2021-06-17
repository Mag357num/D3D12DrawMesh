#include "ActorComponent.h"

void FActorComponent::SetWorldMatrix(const FMatrix& Matrix)
{
	WorldMatrix = Matrix;

	Transform.Quat = glm::quat_cast(Matrix);
	Transform.Translation = FVector(Matrix[3][0], Matrix[3][1], Matrix[3][2]);  // FMatrix[column][row]
	Transform.Scale = FVector(1.f, 1.f, 1.f);

	WorldMatrixDirty = false;
}

void FMeshComponent::SetMaterial(shared_ptr<FMaterialInterface> Mat, uint32 index)
{
	if (Materials.size() < index + 1)
	{
		Materials.resize(index + 1);
	}

	Materials[index] = Mat;
}
