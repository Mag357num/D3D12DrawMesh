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
	shared_ptr<ACharacter> CurrentCharacter; // create by a character manager
	shared_ptr<ACamera> CurrentCamera;
	vector<ACamera> SceneCameras;
	FDirectionLight DirectionLight; // TODO: extend to a array of lights ptr
	vector<AStaticMeshActor> StaticMeshActors;
	vector<ASkeletalMeshActor> SkeletalMeshActors;

public:
	void Tick(StepTimer& Timer);

	// TODO: refactor here. add a Camera controler to manager and store all comera, create camera will be its work. scene only need to pick a camera as current camera.
	void SetCurrentCamera( shared_ptr<ACamera> Cam ) { CurrentCamera = Cam; }
	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, const float& Fov, const float& Width, const float& Height, const float& NearPlane = 1.f, const float& FarPlane = 5000.f) { return CurrentCamera->Init(PositionParam, UpDir, LookAt, Fov, Width, Height, NearPlane, FarPlane); }
	void SetDirectionLight(const FDirectionLight& Light) { DirectionLight = Light; }
	void SetCurrentCharacter(shared_ptr<ACharacter> Character) { CurrentCharacter = Character; };

	// TODO: refactor here. createCharacter should not be Scene's work
	ACamera* GetCurrentCamera() { return CurrentCamera.get(); }
	ACharacter* GetCurrentCharacter() { return CurrentCharacter.get(); }
	FDirectionLight& GetDirectionLight();
	vector<AStaticMeshActor>& GetStaticMeshActors();
	void AddStaticMeshActor( AStaticMeshActor Actor );
};