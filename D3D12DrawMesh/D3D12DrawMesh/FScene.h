#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"

class FScene
{
public:
	FCamera SceneCamera; // TODO: change to std::vector<FCamera> Cameras
	vector<shared_ptr<RHI::FActor>> Actors;

public:
	void UpdateMainCamera(class FEngine* Engine);
	FCamera& GetCurrentCamera() { return SceneCamera; }
	vector<shared_ptr<RHI::FActor>>& GetActors() { return Actors; }
	void UpdateActor() { }
};