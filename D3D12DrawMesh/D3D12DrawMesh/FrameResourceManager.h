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

class FFrameResource
{
public:
	std::vector<FMeshActorFrameResource> MeshActorFrameResources;
	shared_ptr<FHandle> NullTextureHandle;
};

class FFrameResourceManager // TODO: this now is actually ResourceManager, nothing to do with Frame Resource
{
public:
	std::vector<FFrameResource> FrameResources;
public:
	void CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(FScene* Scene, const uint32& FrameIndex);
	void CreateMeshActorFrameResources(FMeshActorFrameResource& MeshActorFrameResource, const FMeshActor& MeshActor);
};