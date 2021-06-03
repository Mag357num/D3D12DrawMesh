#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "StepTimer.h"

class ACharacter;
class ACamera;
class ADirectionalLight;
class APointLight;
class AStaticMeshActor;
class ASkeletalMeshActor;
class FScene
{
private:
	shared_ptr<ACharacter> CurrentCharacter;
	shared_ptr<ACamera> CurrentCamera;
	vector<shared_ptr<ACamera>> SceneCameras; // swap ptr to change camera
	shared_ptr<ADirectionalLight> DirectionalLight;
	vector<shared_ptr<APointLight>> PointLights;
	vector<shared_ptr<AStaticMeshActor>> StaticMeshActors;
	vector<ASkeletalMeshActor> SkeletalMeshActors;

public:
	void Tick(StepTimer& Timer);

	void SetCurrentCharacter( shared_ptr<ACharacter> Character ) { CurrentCharacter = Character; };
	void SetCurrentCamera( shared_ptr<ACamera> Cam ) { CurrentCamera = Cam; }
	void SetDirectionalLight( shared_ptr<ADirectionalLight> Light ) { DirectionalLight = Light; }
	void AddPointLight( shared_ptr<APointLight> Light ) { PointLights.push_back( Light ); }
	void AddStaticMeshActor( shared_ptr<AStaticMeshActor> Actor ) { StaticMeshActors.push_back( Actor ); }

	ACharacter* GetCurrentCharacter() { return CurrentCharacter.get(); }
	ACamera* GetCurrentCamera() { return CurrentCamera.get(); }
	ADirectionalLight* GetDirectionalLight() { return DirectionalLight.get(); }
	vector<shared_ptr<APointLight>> GetPointLights() { return PointLights; }
	vector<shared_ptr<AStaticMeshActor>>& GetStaticMeshActors() { return StaticMeshActors; }
};