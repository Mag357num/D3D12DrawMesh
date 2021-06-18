#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "StepTimer.h"

class ACharacter;
class ACameraActor;
class ADirectionalLight;
class APointLight;
class AStaticMeshActor;
class ASkeletalMeshActor;
class FScene
{
private:
	// all kinds of actors
	shared_ptr<ACameraActor> CurrentCamera;
	shared_ptr<ACharacter> CurrentCharacter;
	vector<shared_ptr<ACameraActor>> CameraActors; // swap ptr to change camera
	vector<shared_ptr<ADirectionalLight>> DirectionalLightActor;
	vector<shared_ptr<APointLight>> PointLightActors;
	vector<shared_ptr<AStaticMeshActor>> StaticMeshActors;
	vector<ASkeletalMeshActor> SkeletalMeshActors;

public:
	void Tick(StepTimer& Timer);

	void SetCurrentCharacter( shared_ptr<ACharacter> Character ) { CurrentCharacter = Character; };
	void AddCamera(shared_ptr<ACameraActor> Cam) { CameraActors.push_back(Cam); }
	void SetCurrentCamera(uint32 Index) { CurrentCamera = CameraActors[Index]; }
	void AddDirectionalLight( shared_ptr<ADirectionalLight> Light ) { DirectionalLightActor.push_back(Light); }
	void AddPointLight( shared_ptr<APointLight> Light ) { PointLightActors.push_back( Light ); }
	void AddStaticMeshActor( shared_ptr<AStaticMeshActor> Actor ) { StaticMeshActors.push_back( Actor ); }

	ACharacter* GetCurrentCharacter() { return CurrentCharacter.get(); }
	ACameraActor* GetCurrentCamera() { return CurrentCamera.get(); }
	vector<shared_ptr<ADirectionalLight>>& GetDirectionalLights() { return DirectionalLightActor; }
	vector<shared_ptr<APointLight>>& GetPointLights() { return PointLightActors; }
	vector<shared_ptr<AStaticMeshActor>>& GetStaticMeshActors() { return StaticMeshActors; }
};