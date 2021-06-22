#include "Light.h"
#include "StaticMesh.h"
#include "AssetManager.h"

FDirectionalLightComponent::FDirectionalLightComponent(const FVector& Pos, const FVector& Direction)
{
	VMatrix_GameThread = glm::lookAtLH(Pos, Pos + Direction * 10.0f, FVector(0.f, 0.f, 1.f));
	SetWorldMatrix(glm::inverse(VMatrix_GameThread));
}

const FMatrix& FDirectionalLightComponent::GetOMatrix_GameThread()
{
	if (ODirty)
	{
		float OrthoHeight = OrthoWidth / AspectRatio;
		OMatrix_GameThread = glm::orthoLH_ZO(-OrthoWidth / 2, OrthoWidth / 2, -OrthoHeight / 2, OrthoHeight / 2, NearPlane, FarPlane);
		ODirty = false;
	}
	return OMatrix_GameThread;
}

const FMatrix& FDirectionalLightComponent::GetOMatrix_RenderThread()
{
	FMatrix GameThread = GetOMatrix_GameThread();
	std::swap(GameThread, OMatrix_RenderThread);
	return OMatrix_RenderThread;
}

ALight::ALight()
{
	shared_ptr<FStaticMeshComponent> Com = make_shared<FStaticMeshComponent>();
	Com->SetStaticMesh(FAssetManager::Get()->LoadStaticMesh(L"Resource\\Mesh\\Sphere_.dat"));
	OwnedComponents.push_back(Com);
}

void ADirectionalLight::Tick(const float& ElapsedSeconds, FLightMoveMode Mode, FVector TargetLocation /*= FVector(0.f, 0.f, 0.f)*/, float Distance /*= 1000.f*/)
{
	switch (Mode)
	{
	case FLightMoveMode::ROTATE_LIGHT:
		Tick_Rotate(ElapsedSeconds, TargetLocation, Distance);
		break;
	case FLightMoveMode::STATIC_LIGHT:
		Tick_Static();
		break;
	default:
		break;
	}
}

void ADirectionalLight::Tick_Rotate(const float& ElapsedSeconds, const FVector& Target, const float& Distance)
{

}

void ADirectionalLight::Tick_Static()
{

}

void FDirectionalLightComponent::SetDirection(const FVector& Dir)
{
	// TODO: low efficiency
	// optimization: https://blog.csdn.net/qq_36537774/article/details/86534009
	const FVector& Eye = GetTransform().Translation;
	SetWorldMatrix(inverse(glm::lookAtLH(Eye, Eye + Dir * 10.0f, FVector(0.f, 0.f, 1.f))));
}

const FVector FDirectionalLightComponent::GetDirection()
{
	// world matrix only contain rotate in 3x3 zone and directional vector wont translate
	// view matrix default use z:FVector4(0, 0, 1, 0) axis as look at
	return GetWorldMatrix() * FVector4(0, 0, 1, 0);
}

const FMatrix& FDirectionalLightComponent::GetViewMatrix_GameThread()
{
	if (VDirty)
	{
		// TODO: compare with use sqt calculate vmatrix
		// t-> eye
		// s-> identity
		// q-> look = quat * (0, 0, 1)
		// vmatrix = lookatLH(eye, look + eye, up)
		VMatrix_GameThread = inverse(WorldMatrix);
		VDirty = false;
	}
	return VMatrix_GameThread;
}

const FMatrix& FDirectionalLightComponent::GetViewMatrix_RenderThread()
{
	FMatrix GameThread = GetViewMatrix_GameThread();
	std::swap(GameThread, VMatrix_RenderThread);
	return VMatrix_RenderThread;
}

FPointLightComponent::FPointLightComponent(const FVector& Pos)
{
	VMatrixs_GameThread[0] = glm::lookAtLH(Pos, Pos + FVector( 1.f,  0.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[1] = glm::lookAtLH(Pos, Pos + FVector(-1.f,  0.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[2] = glm::lookAtLH(Pos, Pos + FVector( 0.f,  1.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[3] = glm::lookAtLH(Pos, Pos + FVector( 0.f, -1.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[4] = glm::lookAtLH(Pos, Pos + FVector( 0.f,  0.f,  1.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[5] = glm::lookAtLH(Pos, Pos + FVector( 0.f,  0.f, -1.f) * 10.0f, FVector(0.f, 0.f, 1.f));

	SetTranslate(Pos);
}

const array<FMatrix, 6>& FPointLightComponent::GetViewMatrixs_GameThread()
{
	if (VDirty)
	{
		const FVector& Pos = GetTransform().Translation;
		VMatrixs_GameThread[0] = glm::lookAtLH(Pos, Pos + FVector(1.f, 0.f, 0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
		VMatrixs_GameThread[1] = glm::lookAtLH(Pos, Pos + FVector(-1.f, 0.f, 0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
		VMatrixs_GameThread[2] = glm::lookAtLH(Pos, Pos + FVector(0.f, 1.f, 0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
		VMatrixs_GameThread[3] = glm::lookAtLH(Pos, Pos + FVector(0.f, -1.f, 0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
		VMatrixs_GameThread[4] = glm::lookAtLH(Pos, Pos + FVector(0.f, 0.f, 1.f) * 10.0f, FVector(0.f, 0.f, 1.f));
		VMatrixs_GameThread[5] = glm::lookAtLH(Pos, Pos + FVector(0.f, 0.f, -1.f) * 10.0f, FVector(0.f, 0.f, 1.f));

		VDirty = false;
	}
	return VMatrixs_GameThread;
}

const array<FMatrix, 6>& FPointLightComponent::GetViewMatrixs_RenderThread()
{
	array<FMatrix, 6> GameThread = GetViewMatrixs_GameThread();
	std::swap(GameThread, VMatrixs_RenderThread);
	return VMatrixs_RenderThread;
}

const FMatrix& FPointLightComponent::GetPMatrix_GameThread()
{
	if (PDirty)
	{
		PMatrix_GameThread = glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
		PDirty = false;
	}
	return PMatrix_GameThread;
}

const FMatrix& FPointLightComponent::GetPMatrix_RenderThread()
{
	FMatrix GameThread = GetPMatrix_GameThread();
	std::swap( GameThread, PMatrix_RenderThread );
	return PMatrix_RenderThread;
}

void APointLight::Tick( const float& ElapsedSeconds )
{

}
