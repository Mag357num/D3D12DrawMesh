#pragma once
#include "stdafx.h"
#include "Actor.h"

enum class FLightMoveMode
{
	ROTATE_LIGHT = 0,
	STATIC_LIGHT = 1
};

class FLightComponent : public FSceneComponent
{
protected:
	FVector4 Color;
	float Intensity;
	float AspectRatio = 1.7777777f;
	float NearPlane = 1.f;
	float FarPlane = 5000.f;

public:
	FLightComponent() = default;

	void SetColor(const FVector4& C) { Color = C; }
	void SetIntensity(const float& I) { Intensity = I; }
	virtual void SetClipPlane(const float& InFarPlane, const float& InNear) = 0;
	virtual void SetAspectRatio(const float& InAspectRatio) = 0;

	const FVector4& GetColor() { return Color; }
	const float& GetIntensity() { return Intensity; }

protected:
	void SetClipPlane_Base(const float& InFarPlane, const float& InNear) { FarPlane = InFarPlane; NearPlane = InNear; }
	void SetAspectRatio_Base(const float& InAspectRatio) { AspectRatio = InAspectRatio; }
};

class ALight : public AActor
{
protected:
	// movement parameter
	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	float AngularVelocity;

public:
	ALight();
	~ALight() = default;
};

class FDirectionalLightComponent : public FLightComponent
{
private:
	// Secondary data, need to refresh depent on dirty
	bool VDirty = true;
	FMatrix VMatrix_GameThread;
	FMatrix VMatrix_RenderThread;

	// directional light shadowmap is ortho projection, Secondary data, need to refresh depent on dirty
	bool ODirty = true;
	FMatrix OMatrix_GameThread;
	FMatrix OMatrix_RenderThread;

	float OrthoWidth;

public:
	FDirectionalLightComponent() { Type = EComponentType::DIRECTIONALLIGHT_COMPONENT; };
	virtual ~FDirectionalLightComponent() = default;
	FDirectionalLightComponent(const FVector& Pos, const FVector& Direction);

	void SetDirection(const FVector& Dir);
	void SetOrthoWidth(const float& Width) { OrthoWidth = Width; ODirty = true; }
	virtual void SetClipPlane(const float& InFarPlane, const float& InNear) override { SetClipPlane_Base(InFarPlane, InNear); ODirty = true; }
	virtual void SetAspectRatio(const float& InAspectRatio) override { SetAspectRatio_Base(InAspectRatio); ODirty = true; };

	const FVector GetDirection();
	const FMatrix& GetViewMatrix_GameThread();
	const FMatrix& GetViewMatrix_RenderThread();
	const FMatrix& GetOMatrix_GameThread();
	const FMatrix& GetOMatrix_RenderThread();

	void SetScale(const FVector& Scale) { SetScale_Base(Scale); VDirty = true; }
	void SetQuat(const FQuat& Quat) { SetQuat_Base(Quat); VDirty = true; }
	void SetTranslate(const FVector& Translate) { SetTranslate_Base(Translate); VDirty = true; }
	void SetTransform(const FTransform& Trans) { SetTransform_Base(Trans); VDirty = true; }
	void SetWorldMatrix(const FMatrix& Matrix) { SetWorldMatrix_Base(Matrix); VDirty = true; }
};

class ADirectionalLight : public ALight
{
private:
	FDirectionalLightComponent* DirectionalLightComponent;

public:
	ADirectionalLight(shared_ptr<FDirectionalLightComponent> DLight) { RootComponent = DLight; DirectionalLightComponent = DLight.get(); AddOwnedComponent(DLight); }

	virtual void Tick(const float& ElapsedSeconds, FLightMoveMode Mode, FVector TargetLocation = FVector(0.f, 0.f, 0.f), float Distance = 1000.f);
	virtual void Tick_Rotate(const float& ElapsedSeconds, const FVector& Target, const float& Distance);
	virtual void Tick_Static();

	FDirectionalLightComponent* GetDLightComponent() { return DirectionalLightComponent; }
};

class FPointLightComponent : public FLightComponent
{
private:
	const FVector Attenuation = FVector(1.0f, 0.09f, 0.032f); // constant, linear, quadratic
	float AttenuationRadius;
	float SourceRadius;

	// Secondary data, need to refresh depent on dirty
	bool VDirty = true;
	array<FMatrix, 6> VMatrixs_GameThread; // 6 vmatrix and 1 pmatrix for cube map
	array<FMatrix, 6> VMatrixs_RenderThread;

	// point light shadowmap is perspective projection, Secondary data, need to refresh depent on dirty
	bool PDirty = true;
	FMatrix PMatrix_GameThread;
	FMatrix PMatrix_RenderThread;

	float Fov = 90.f;

public:
	FPointLightComponent() { Type = EComponentType::POINTLIGHT_COMPONENT; };
	virtual ~FPointLightComponent() = default;
	FPointLightComponent(const FVector& Pos);

	void SetTranslate(const FVector& Translate) { SetTranslate_Base(Translate); VDirty = true; }
	void SetTransform(const FTransform& Trans) { SetTransform_Base(Trans); VDirty = true; }
	void SetWorldMatrix(const FMatrix& Matrix) { SetWorldMatrix_Base(Matrix); VDirty = true; }
	void SetFov(const float& F) { Fov = F; PDirty = true; }
	virtual void SetClipPlane(const float& InFarPlane, const float& InNear) override { SetClipPlane_Base(InFarPlane, InNear); PDirty = true; }
	virtual void SetAspectRatio(const float& InAspectRatio) override { SetAspectRatio_Base(InAspectRatio); PDirty = true; };
	void SetAttenuationRadius(const float& InAttenuationRadius) { AttenuationRadius = InAttenuationRadius; }
	void SetSourceRadius(const float& InSourceRadius) { SourceRadius = InSourceRadius; }

	const FVector& GetAttenuation() { return Attenuation; }
	const array<FMatrix, 6>& GetViewMatrixs_GameThread();
	const array<FMatrix, 6>& GetViewMatrixs_RenderThread();
	const FMatrix& GetPMatrix_GameThread();
	const FMatrix& GetPMatrix_RenderThread();
};

class APointLight : public ALight
{
private:
	FPointLightComponent* PointLightComponent;

public:
	APointLight(shared_ptr<FPointLightComponent> PLight) { RootComponent = PLight; PointLightComponent = PLight.get(); AddOwnedComponent(PLight); }

	virtual void Tick(const float& ElapsedSeconds);

	FPointLightComponent* GetPLightComponent() { return PointLightComponent; }
};