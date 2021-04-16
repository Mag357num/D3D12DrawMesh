#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "SimpleCamera.h"
#include "StaticMeshActor.h"

class FScene
{
public:
	//vector<shared_ptr<RHI::FIndividual>> Individuals; //TODO: to remove
	//uint32 IndividualsNum; //TODO: to remove

	FCamera SceneCamera; // TODO: change to std::vector<FCamera> Cameras
	vector<FMeshActor> MeshActors;

public:
	void UpdateMainCamera(class FEngine* Engine);
	FCamera& GetCurrentCamera() { return SceneCamera; }

	//vector<shared_ptr<RHI::FIndividual>>& GetIndividuals() { return Individuals; } //TODO: to remove
};