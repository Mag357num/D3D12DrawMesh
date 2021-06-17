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
	// all kinds of actors
	shared_ptr<ACamera> CurrentCamera;
	shared_ptr<ACharacter> CurrentCharacter;
	shared_ptr<ADirectionalLight> DirectionalLightActor;
	vector<shared_ptr<ACamera>> CameraActors; // swap ptr to change camera
	vector<shared_ptr<APointLight>> PointLightActors;
	vector<shared_ptr<AStaticMeshActor>> StaticMeshActors;
	vector<ASkeletalMeshActor> SkeletalMeshActors;

public:
	void Tick(StepTimer& Timer);

	void SetCurrentCharacter( shared_ptr<ACharacter> Character ) { CurrentCharacter = Character; };
	void SetCurrentCamera( shared_ptr<ACamera> Cam ) { CurrentCamera = Cam; }
	void SetDirectionalLight( shared_ptr<ADirectionalLight> Light ) { DirectionalLightActor = Light; }
	void AddPointLight( shared_ptr<APointLight> Light ) { PointLightActors.push_back( Light ); }
	void AddStaticMeshActor( shared_ptr<AStaticMeshActor> Actor ) { StaticMeshActors.push_back( Actor ); }

	ACharacter* GetCurrentCharacter() { return CurrentCharacter.get(); }
	ACamera* GetCurrentCamera() { return CurrentCamera.get(); }
	ADirectionalLight* GetDirectionalLight() { return DirectionalLightActor.get(); }
	vector<shared_ptr<APointLight>>& GetPointLights() { return PointLightActors; }
	vector<shared_ptr<AStaticMeshActor>>& GetStaticMeshActors() { return StaticMeshActors; }
};