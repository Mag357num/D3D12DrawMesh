#include "Light.h"
#include "StaticMesh.h"
#include "AssetManager.h"

ADirectionalLight::ADirectionalLight(const FVector& Pos, const FVector& Direction, const FVector& Color)
{
	this->Color = Color;

	VMatrix_GameThread = glm::lookAtLH(Pos, Pos + Direction * 10.0f, FVector(0.f, 0.f, 1.f));
	VDirty = false;
	SetWorldMatrix(glm::inverse(VMatrix_GameThread));
}

void ADirectionalLight::SetOrthoParam(float L, float R, float B, float T, float N, float F)
{
	Left = L;
	Right = R;
	Bottom = B;
	Top = T;
	NearPlane = N;
	FarPlane = F;
	ODirty = true;
}

const FMatrix& ADirectionalLight::GetOMatrix_GameThread()
{
	if (ODirty)
	{
		OMatrix_GameThread = glm::orthoLH_ZO(Left, Right, Bottom, Top, NearPlane, FarPlane);
		ODirty = false;
	}
	return OMatrix_GameThread;
}

const FMatrix& ADirectionalLight::GetOMatrix_RenderThread()
{
	FMatrix GameThread = GetOMatrix_GameThread();
	std::swap(GameThread, OMatrix_RenderThread);
	return OMatrix_RenderThread;
}

ALight::ALight()
{
	shared_ptr<FStaticMeshComponent> Com = make_shared<FStaticMeshComponent>();
	Com->SetStaticMesh(FAssetManager::Get()->LoadStaticMesh(L"Resource\\Sphere_.dat"));
	Components.push_back(Com);
}

void ALight::SetQuat(const FQuat& Quat)
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Light dont have component!");
	}
	else
	{
		Components[0]->SetQuat(Quat);
		VDirty = true;
	}
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

void ALight::SetTranslate(const FVector& Trans)
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Light dont have component!");
	}
	else
	{
		Components[0]->SetTranslate(Trans);
		VDirty = true;
	}
}

void ADirectionalLight::SetDirection(const FVector& Dir)
{
	// TODO: low efficiency
	// optimization: https://blog.csdn.net/qq_36537774/article/details/86534009
	const FVector& Eye = GetTransform().Translation;
	const FVector Up(0.f, 0.f, 1.f);
	SetWorldMatrix(inverse(glm::lookAtLH(Eye, Eye + Dir * 10.0f, Up)));
}

void ALight::SetWorldMatrix(const FMatrix& W)
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Camera dont have component!");
	}
	else
	{
		Components[0]->SetWorldMatrix(W);
		VDirty = true;
	}
}

const FTransform& ALight::GetTransform()
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Light dont have component!");
	}
	else
	{
		return Components[0]->GetTransform();
	}
}

const FMatrix& ALight::GetWorldMatrix()
{
	if (Components.size() == 0)
	{
		throw std::exception("ERROR: Light dont have component!");
	}
	else
	{
		return Components[0]->GetWorldMatrix();
	}
}

const FVector ADirectionalLight::GetDirection()
{
	// world matrix only contain rotate in 3x3 zone and directional vector wont translate
	// view matrix default use z:FVector4(0, 0, 1, 0) axis as look at
	return GetWorldMatrix() * FVector4(0, 0, 1, 0);
}

const FMatrix& ADirectionalLight::GetViewMatrix_GameThread()
{
	if (VDirty)
	{
		// TODO: compare with use sqt calculate vmatrix
		// t-> eye
		// s-> identity
		// q-> look = quat * (0, 0, 1)
		// vmatrix = lookatLH(eye, look + eye, up)
		VMatrix_GameThread = inverse(Components[0]->GetWorldMatrix());
		VDirty = false;
	}
	return VMatrix_GameThread;
}

const FMatrix& ADirectionalLight::GetViewMatrix_RenderThread()
{
	FMatrix GameThread = GetViewMatrix_GameThread();
	std::swap(GameThread, VMatrix_RenderThread);
	return VMatrix_RenderThread;
}

void ALight::SetStaticMeshComponent(shared_ptr<FStaticMeshComponent> Com)
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

FStaticMeshComponent* ALight::GetStaticMeshComponent()
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

APointLight::APointLight(const FVector& Pos, const FVector& Color)
{
	this->Color = Color;

	VMatrixs_GameThread[0] = glm::lookAtLH(Pos, Pos + FVector( 1.f,  0.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[1] = glm::lookAtLH(Pos, Pos + FVector(-1.f,  0.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[2] = glm::lookAtLH(Pos, Pos + FVector( 0.f,  1.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[3] = glm::lookAtLH(Pos, Pos + FVector( 0.f, -1.f,  0.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[4] = glm::lookAtLH(Pos, Pos + FVector( 0.f,  0.f,  1.f) * 10.0f, FVector(0.f, 0.f, 1.f));
	VMatrixs_GameThread[5] = glm::lookAtLH(Pos, Pos + FVector( 0.f,  0.f, -1.f) * 10.0f, FVector(0.f, 0.f, 1.f));

	VDirty = false;
	SetTranslate(Pos);
}

const array<FMatrix, 6>& APointLight::GetViewMatrixs_GameThread()
{
	if (VDirty)
	{
		FVector Pos = GetTransform().Translation;

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

const array<FMatrix, 6>& APointLight::GetViewMatrixs_RenderThread()
{
	array<FMatrix, 6> GameThread = GetViewMatrixs_GameThread();
	std::swap(GameThread, VMatrixs_RenderThread);
	return VMatrixs_RenderThread;
}

const FMatrix& APointLight::GetPMatrix_GameThread()
{
	if (PDirty)
	{
		PMatrix_GameThread = glm::perspectiveFovLH_ZO(Fov, AspectRatio, 1.0f, NearPlane, FarPlane);
		PDirty = false;
	}
	return PMatrix_GameThread;
}

const FMatrix& APointLight::GetPMatrix_RenderThread()
{
	FMatrix GameThread = GetPMatrix_GameThread();
	std::swap( GameThread, PMatrix_RenderThread );
	return PMatrix_RenderThread;
}

void APointLight::Tick( const float& ElapsedSeconds )
{

}
