#pragma once
#include "stdafx.h"
#include "Camera.h"

enum class FLightMoveMode
{
	ROTATE_LIGHT = 0,
	STATIC_LIGHT = 1
};

//struct FLight
//{
//	FVector Position = { 450.f, 0.f, 450.f };
//	bool IsChanged = true;
//	FVector Color = { 1.f, 1.f, 1.f };
//	float Intensity = 1.0f;
//};
//
//struct FDirectionLight : public FLight
//{
//	FVector Dir = { -1.f, 0.f, -1.f };
//	FMatrix VPMatrix;
//
//	FMatrix GetLightVPMatrix();
//};
//
//struct FPointLight : public FLight
//{
//	struct
//	{
//		float Constant;
//		float Linear;
//		float Exp;
//	} Attenuation;
//};

class ALight : public AActor
{
protected:
	// base param
	FVector Color = { 1.f, 1.f, 1.f };
	float Intensity = 1.0f;

	// view matrix depend on position, so define in the alight
	bool VDirty = true;
	FMatrix VMatrix_GameThread;
	FMatrix VMatrix_RenderThread;

	// movement parameter
	float MoveSpeed = 300.0f;
	float TurnSpeed = 1.570796327f;
	float MouseSensibility = 0.01f;
	float AngularVelocity;

public:
	ALight();
	~ALight() = default;

	virtual const FVector& GetColor() const = 0;
	virtual const float& GetIntensity() const = 0;
	virtual void SetIntensity(const float& Inten) = 0;
	virtual void SetColor(const FVector& C) = 0;

	void SetQuat(const FQuat& Quat);
	void SetTranslate(const FVector& Trans);
	void SetWorldMatrix(const FMatrix& W);
	const FTransform& GetTransform();
	const FMatrix& GetWorldMatrix();

	void SetDirection(const FVector& Dir);
	const FVector GetDirection();

	const FMatrix& GetViewMatrix_GameThread();
	const FMatrix& GetViewMatrix_RenderThread();
};

class ADirectionLight : public ALight
{
private:

	// directional light shadowmap is ortho projection
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
	ADirectionLight() = delete;
	ADirectionLight(const FVector& Pos, const FVector& Direction, const FVector& Color);
	~ADirectionLight() = default;

	virtual const FVector& GetColor() const override { return Color; };
	virtual const float& GetIntensity() const override { return Intensity; }
	virtual void SetIntensity(const float& Inten) override { Intensity = Inten; }
	virtual void SetColor(const FVector& C) override { Color = C; }

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
	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;

	// point light shadowmap is perspective projection
	bool PDirty = true;
	FMatrix PMatrix;

	float Fov = 90.f;
	float AspectRatio = 1.7777777f;
	float NearPlane = 1.f;
	float FarPlane = 5000.f;

public:
	virtual const FVector& GetColor() const override { return Color; };
	virtual const float& GetIntensity() const override { return Intensity; }
	virtual void SetIntensity(const float& Inten) override { Intensity = Inten; }
	virtual void SetColor(const FVector& C) override { Color = C; }

	void SetPerspParam(float Fov, float AspectRatio, float Near, float Far) { this->Fov = Fov; this->AspectRatio = AspectRatio; NearPlane = Near; FarPlane = Far; PDirty = true; }

	const FMatrix& GetPMatrix_GameThread();
	const FMatrix& GetPMatrix_RenderThread();
};