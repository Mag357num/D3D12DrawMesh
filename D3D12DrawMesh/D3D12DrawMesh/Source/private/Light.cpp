#include "Light.h"

FMatrix FDirectionLight::GetLightVPMatrix()
{
	if (IsChanged)
	{
		FVector LightPos = Position;
		FVector LightDirNored = glm::normalize( Dir );
		FVector LightTarget = LightPos + LightDirNored;
		FVector LightUpDir = { 0.f, 0.f, 1.f };  // TODO: hard coding

		FMatrix LightView = glm::lookAtLH( LightPos, LightTarget, LightUpDir );
		FMatrix LightProj = glm::orthoLH_ZO( -700.f, 700.f, -700.f, 700.f, 1.0f, 3000.0f ); // TODO: hard coding
		VPMatrix = LightProj * LightView;

		IsChanged = false;
	}
	return VPMatrix;
}
