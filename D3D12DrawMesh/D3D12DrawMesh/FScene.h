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
	FCamera& GetCurrentCamera() { return SceneCamera; }
};