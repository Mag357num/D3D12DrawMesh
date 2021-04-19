#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"
#include "StaticMeshActor.h"
#include "Light.h"

class FScene
{
public:
	FCamera SceneCamera; // TODO: change to std::vector<FCamera> Cameras
	vector<FMeshActor> MeshActors;
	FDirectionLight DirectionLight; // TODO: extend to a array of lights

public:
	void UpdateMainCamera(class FEngine* Engine);
	void SetCurrentCamera(const FVector& PositionParam, const FVector& UpDir, const FVector& LookAt, float Fov, float AspectRatio) { return SceneCamera.Init(PositionParam, UpDir, LookAt, Fov, AspectRatio); }
	FCamera& GetCurrentCamera() { return SceneCamera; }
};