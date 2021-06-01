#include "ActorComponent.h"

void FActorComponent::SetMatrix(const FMatrix& Matrix)
{
	TransMatrix = Matrix;

	Transform.Quat = glm::quat_cast(Matrix);
	Transform.Translation = FVector(Matrix[3][0], Matrix[3][1], Matrix[3][2]);  // FMatrix[column][row]
	Transform.Scale = FVector(1, 1, 1);

	Dirty = false;
}

