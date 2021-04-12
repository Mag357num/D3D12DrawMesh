#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"

class FScene
{
public:
	FCamera MainCamera; // TODO: change to std::vector<FCamera> Cameras
	vector<shared_ptr<RHI::FActor>> Actors;

public:
	const FCamera& GetCurrentCamera() const { return MainCamera; }
	vector<shared_ptr<RHI::FActor>>& GetActors() { return Actors; }
	void UpdateActor();
};