#pragma once
#include "stdafx.h"
#include "Actor.h"

enum class FLightMoveMode
{
	ROTATE_LIGHT = 0,
	STATIC_LIGHT = 1
};

class ALight : public AActor
{
protected:
	// base param
	FVector Color = { 1.f, 1.f, 1.f };
	FVector Ambient = { 0.02f, 0.02f, 0.02f };
	FVector Diffuse = { 0.3f , 0.3f , 0.3f };
	FVector Specular = { 1.5f, 1.5f, 1.5f };

	// movement parameter
	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	float AngularVelocity;

	// view matrix depend on the position of light, so define in ALight
	bool VDirty = true;

	// camera change flag
	uint32 LightDirtyCount = 3;

public:
	ALight();
	~ALight() = default;

	void SetColor(const FVector& C) { Color = C; }
	void SetAmbient(const FVector& A) { Ambient = A; }
	void SetDiffuse(const FVector& D) { Diffuse = D; }
	void SetSpecular(const FVector& S) { Specular = S; }
	const FVector& GetColor() const { return Color; };
	const FVector& GetAmbient() { return Ambient; }
	const FVector& GetDiffuse() { return Diffuse; }
	const FVector& GetSpecular() { return Specular; }

	void SetQuat(const FQuat& Quat);
	void SetTranslate(const FVector& Trans);
	void SetWorldMatrix(const FMatrix& W);

	const FTransform& GetTransform();
	const FMatrix& GetWorldMatrix();

	const bool IsDirty() const { return LightDirtyCount != 0; }
	void DecreaseDirty() { LightDirtyCount--; }

	void SetStaticMeshComponent(shared_ptr<class FStaticMeshComponent> Com);
	class FStaticMeshComponent* GetStaticMeshComponent();
};

class ADirectionalLight : public ALight
{
private:
	// Secondary data, need to refresh depent on dirty
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
	ADirectionalLight() = delete;
	ADirectionalLight(const FVector& Pos, const FVector& Direction, const FVector& Color = FVector(1, 1, 1));
	~ADirectionalLight() = default;

	void SetDirection(const FVector& Dir);
	const FVector GetDirection();

	const FMatrix& GetViewMatrix_GameThread();
	const FMatrix& GetViewMatrix_RenderThread();

	void SetOrthoParam(float L, float R, float B, float T, float N, float F);

	const FMatrix& GetOMatrix_GameThread();
	const FMatrix& GetOMatrix_RenderThread();

	virtual void Tick(const float& ElapsedSeconds, FLightMoveMode Mode, FVector TargetLocation = FVector(0.f, 0.f, 0.f), float Distance = 1000.f);
	virtual void Tick_Rotate(const float& ElapsedSeconds, const FVector& Target, const float& Distance);
	virtual void Tick_Static();
};

class APointLight : public ALight
{
private:
	float Constant;
	float Linear;
	float Quadratic;

	// Secondary data, need to refresh depent on dirty
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
	APointLight() = delete;
	APointLight(const FVector& Pos, const FVector& Color = FVector(1, 1, 1));
	~APointLight() = default;

	const array<FMatrix, 6>& GetViewMatrixs_GameThread();
	const array<FMatrix, 6>& GetViewMatrixs_RenderThread();

	void SetPerspParam(float Fov, float AspectRatio, float Near, float Far) { this->Fov = Fov; this->AspectRatio = AspectRatio; NearPlane = Near; FarPlane = Far; PDirty = true; }

	const FMatrix& GetPMatrix_GameThread();
	const FMatrix& GetPMatrix_RenderThread();

	virtual void Tick( const float& ElapsedSeconds);
};