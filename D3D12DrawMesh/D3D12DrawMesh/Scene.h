#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"
#include "StaticMesh.h"
#include "Light.h"
#include "Character.h"
#include "StepTimer.h"

class FScene
{
private:
	FCamera CurrentCamera; // TODO: change to std::vector<FCamera> Cameras
	FDirectionLight DirectionLight; // TODO: extend to a array of lights
	vector<AStaticMeshActor> StaticMeshActors;
	ACharacter Character;

public:
	void Tick(StepTimer& Timer);

	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float AspectRatio) { return CurrentCamera.Init(PositionParam, UpDir, LookAt, Fov, AspectRatio); }
	void SetDirectionLight(const FDirectionLight& Light) { DirectionLight = Light; }

	FCamera& GetCurrentCamera() { return CurrentCamera; }
	ACharacter& GetCharacter() { return Character; }
	const FDirectionLight& GetDirectionLight() const { return DirectionLight; }
	vector<AStaticMeshActor>& GetStaticMeshActors() { return StaticMeshActors; }
};