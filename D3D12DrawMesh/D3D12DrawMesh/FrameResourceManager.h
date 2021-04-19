#pragma once
#include "RHIResource.h"
#include "FScene.h"
#include "stdafx.h"

using namespace RHI;

struct FMeshActorFrameResource
{
	shared_ptr<FMesh> MeshToRender;
	shared_ptr<FMeshRes> MeshResToRender;
	uint32 MeshActorResIndex;

	vector<FTexture> TexturesToRender;
	string Describe;
};

struct FFrameResource
{
	std::vector<FMeshActorFrameResource> MeshActorFrameResources;
};

class FFrameResourceManager // TODO: this now is actually ResourceManager, nothing to do with Frame Resource
{
public:
	std::vector<FFrameResource> FrameResources;
	//shared_ptr<FScene> FrameManagerScene; // TODO: remove
public:
	void CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(FScene* Scene, const uint32& FrameIndex);
	void CreateMeshActorFrameResources(FMeshActorFrameResource& MeshActorFrameResource, const FMeshActor& MeshActor);
};