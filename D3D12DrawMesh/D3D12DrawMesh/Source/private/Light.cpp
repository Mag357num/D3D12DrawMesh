#include "Light.h"
#include "StaticMesh.h"

ADirectionLight::ADirectionLight(const FVector& Pos, const FVector& Direction, const FVector& Color)
{
	this->Color = Color;

	VMatrix_GameThread = glm::lookAtLH(Pos, Pos + Direction * 10.0f, FVector(0.f, 0.f, 1.f));
	VDirty = false;
	SetWorldMatrix(glm::inverse(VMatrix_GameThread));
}

void ADirectionLight::SetOrthoParam(float L, float R, float B, float T, float N, float F)
{
	Left = L;
	Right = R;
	Bottom = B;
	Top = T;
	NearPlane = N;
	FarPlane = F;
	ODirty = true;
}

const FMatrix& ADirectionLight::GetOMatrix_GameThread()
{
	if (ODirty)
	{
		OMatrix_GameThread = glm::orthoLH_ZO(Left, Right, Bottom, Top, NearPlane, FarPlane);
		ODirty = false;
	}
	return OMatrix_GameThread;
}

const FMatrix& ADirectionLight::GetOMatrix_RenderThread()
{
	FMatrix GameThread = GetOMatrix_GameThread();
	std::swap(GameThread, OMatrix_RenderThread);
	return OMatrix_RenderThread;
}

ALight::ALight()
{
	Components.push_back(make_shared<FStaticMeshComponent>());
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

void ADirectionLight::Tick(const float& ElapsedSeconds, FLightMoveMode Mode, FVector TargetLocation /*= FVector(0.f, 0.f, 0.f)*/, float Distance /*= 1000.f*/)
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

void ADirectionLight::Tick_Rotate(const float& ElapsedSeconds, const FVector& Target, const float& Distance)
{

}

void ADirectionLight::Tick_Static()
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

void ALight::SetDirection(const FVector& Dir)
{
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

const FVector ALight::GetDirection()
{
	// world matrix only contain rotate in 3x3 zone and directional vector wont translate
	// view matrix default use z:FVector4(0, 0, 1, 0) axis as look at
	return GetWorldMatrix() * FVector4(0, 0, 1, 0);
}

const FMatrix& ALight::GetViewMatrix_GameThread()
{
	if (VDirty)
	{
		VMatrix_GameThread = inverse(Components[0]->GetWorldMatrix());
		VDirty = false;
	}
	return VMatrix_GameThread;
}

const FMatrix& ALight::GetViewMatrix_RenderThread()
{
	FMatrix GameThread = GetViewMatrix_GameThread();
	std::swap(GameThread, VMatrix_RenderThread);
	return VMatrix_RenderThread;
}