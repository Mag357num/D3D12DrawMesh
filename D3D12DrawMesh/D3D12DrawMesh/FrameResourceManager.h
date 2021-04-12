#pragma once
#include "RHIResource.h"
#include "stdafx.h"

using namespace RHI;

struct FActorRenderResource
{
	std::shared_ptr<FMesh> Mesh;
	std::shared_ptr<FMeshRes> MeshRes;
};

struct FFrameResource
{
	std::vector<FActorRenderResource> ActorRenderResources;
};

class FFrameResourceManager
{
public:
	std::vector<FFrameResource> FrameResources;
};