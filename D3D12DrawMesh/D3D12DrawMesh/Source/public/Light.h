#pragma once
#include "stdafx.h"
#include "Actor.h"

enum class FLightMoveMode
{
	ROTATE_LIGHT = 0,
	STATIC_LIGHT = 1
};

class FLightComponent : public FActorComponent
{
private:
	FVector Color;
	float Intensity;

public:
	void SetColor(const FVector& C) { Color = C; }
	const FVector& GetColor() { return Color; }

	void SetIntensity(const float& I) { Intensity = I; }
	const float& GetIntensity() { return Intensity; }
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

class FDirectionalLIghtComponent : public FLightComponent
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

	float Left;
	float Right;
	float Bottom;
	float Top;
	float NearPlane;
	float FarPlane;

public:
	FDirectionalLIghtComponent() = delete;
	~FDirectionalLIghtComponent() = default;
	FDirectionalLIghtComponent(const FVector& Pos, const FVector& Direction);

	void SetDirection(const FVector& Dir);
	void SetOrthoParam(float L, float R, float B, float T, float N, float F);

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
	FDirectionalLIghtComponent* const DirectionalLightComponent = Components[0]->As<FDirectionalLIghtComponent>();

public:
	virtual void Tick(const float& ElapsedSeconds, FLightMoveMode Mode, FVector TargetLocation = FVector(0.f, 0.f, 0.f), float Distance = 1000.f);
	virtual void Tick_Rotate(const float& ElapsedSeconds, const FVector& Target, const float& Distance);
	virtual void Tick_Static();

	FDirectionalLIghtComponent* GetRootComponent() { return DirectionalLightComponent; }
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
	float AspectRatio = 1.7777777f;
	float NearPlane = 1.f;
	float FarPlane = 5000.f;

public:
	FPointLightComponent() = delete;
	~FPointLightComponent() = default;
	FPointLightComponent(const FVector& Pos);

	void SetTranslate(const FVector& Translate) { SetTranslate_Base(Translate); VDirty = true; }
	void SetTransform(const FTransform& Trans) { SetTransform_Base(Trans); VDirty = true; }
	void SetWorldMatrix(const FMatrix& Matrix) { SetWorldMatrix_Base(Matrix); VDirty = true; }
	void SetPerspParam(float Fov, float AspectRatio, float Near, float Far) { this->Fov = Fov; this->AspectRatio = AspectRatio; NearPlane = Near; FarPlane = Far; PDirty = true; }

	const FVector& GetAttenuation() { return Attenuation; }
	const array<FMatrix, 6>& GetViewMatrixs_GameThread();
	const array<FMatrix, 6>& GetViewMatrixs_RenderThread();
	const FMatrix& GetPMatrix_GameThread();
	const FMatrix& GetPMatrix_RenderThread();
};

class APointLight : public ALight
{
private:
	FPointLightComponent* const PointLightComponent = Components[0]->As<FPointLightComponent>();
public:
	virtual void Tick(const float& ElapsedSeconds);

	FPointLightComponent* GetRootComponent() { return PointLightComponent; }
};