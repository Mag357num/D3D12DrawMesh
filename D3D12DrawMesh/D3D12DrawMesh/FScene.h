#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"
#include "StaticMeshActor.h"
#include "Light.h"

class FScene
{
private:
	FCamera CurrentCamera; // TODO: change to std::vector<FCamera> Cameras
	FDirectionLight DirectionLight; // TODO: extend to a array of lights
	vector<FMeshActor> MeshActors;

public:
	void UpdateMainCamera(class FEngine* Engine);
	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float AspectRatio) { return CurrentCamera.Init(PositionParam, UpDir, LookAt, Fov, AspectRatio); }
	FCamera& GetCurrentCamera() { return CurrentCamera; }
	FDirectionLight& GetDirectionLight() { return DirectionLight; }
	vector<FMeshActor>& GetMeshActors() { return MeshActors; }
};