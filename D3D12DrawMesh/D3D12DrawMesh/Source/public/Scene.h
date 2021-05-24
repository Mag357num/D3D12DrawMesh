#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "Camera.h"
#include "StaticMesh.h"
#include "Light.h"
#include "Character.h"
#include "StepTimer.h"

class FScene
{
private:
	ACamera CurrentCamera; // TODO: change to std::vector<FCamera> Cameras
	FDirectionLight DirectionLight; // TODO: extend to a array of lights
	vector<AStaticMeshActor> StaticMeshActors;
	shared_ptr<ACharacter> CurrentCharacter; // create by a character manager

public:
	void Tick(StepTimer& Timer);

	// TODO: refactor here. add a Camera controler to manager and store all comera, create camera will be its work. scene only need to pick a camera as current camera.
	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float Width, float Height) { return CurrentCamera.Init(PositionParam, UpDir, LookAt, Fov, Width, Height); }
	void SetDirectionLight(const FDirectionLight& Light) { DirectionLight = Light; }
	void SetCharacter(shared_ptr<ACharacter> Character) { CurrentCharacter = Character; };

	// TODO: refactor here. createCharacter should not be Scene's work

	ACamera& GetCurrentCamera() { return CurrentCamera; }
	ACharacter* GetCharacter() { return CurrentCharacter.get(); }
	const FDirectionLight& GetDirectionLight() const { return DirectionLight; }
	vector<AStaticMeshActor>& GetStaticMeshActors() { return StaticMeshActors; }
};