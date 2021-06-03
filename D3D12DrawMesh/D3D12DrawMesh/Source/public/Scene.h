#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "Camera.h"
#include "Light.h"
#include "Character.h"
#include "StepTimer.h"
#include "StaticMesh.h"
#include "SkeletalMesh.h"

class FScene
{
private:
	shared_ptr<ACharacter> CurrentCharacter;
	shared_ptr<ACamera> CurrentCamera;
	vector<shared_ptr<ACamera>> SceneCameras;
	shared_ptr<ADirectionLight> DirectionLight;
	vector<shared_ptr<AStaticMeshActor>> StaticMeshActors;
	vector<ASkeletalMeshActor> SkeletalMeshActors;

public:
	void Tick(StepTimer& Timer);

	void SetCurrentCamera( shared_ptr<ACamera> Cam ) { CurrentCamera = Cam; }
	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.f, const float& FarPlane = 5000.f) { return CurrentCamera->Init(PositionParam, UpDir, LookAt, Fov, Width, Height, NearPlane, FarPlane); }
	void SetDirectionLight(shared_ptr<ADirectionLight> Light);
	void SetCurrentCharacter(shared_ptr<ACharacter> Character) { CurrentCharacter = Character; };

	ACamera* GetCurrentCamera() { return CurrentCamera.get(); }
	ACharacter* GetCurrentCharacter() { return CurrentCharacter.get(); }
	ADirectionLight* GetDirectionLight();
	vector<shared_ptr<AStaticMeshActor>>& GetStaticMeshActors();
	void AddStaticMeshActor(shared_ptr<AStaticMeshActor> Actor );
};