#pragma once
#include "RHIResource.h"
#include "FScene.h"
#include "stdafx.h"

using namespace RHI;

struct FMeshActorFrameRes
{
	shared_ptr<FMesh> Mesh;
	shared_ptr<FMeshRes> MeshRes;

	vector<FTexture> Textures;
};

class FFrameResource
{
public:
	std::vector<FMeshActorFrameRes> MeshActorFrameReses;
	
	uint32 ShadowMapSize = 8192;
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FSampler> ClampSampler;

	shared_ptr<FMesh> PostProcessTriangle;
	shared_ptr<FMeshRes> PostProcessTriangleRes;

	shared_ptr<FTexture> SceneColor;
	shared_ptr<FTexture> BloomSetup;
	shared_ptr<FTexture> Bloomdown8;
	shared_ptr<FTexture> Bloomdown16;
	shared_ptr<FTexture> Bloomdown32;
	shared_ptr<FTexture> Bloomdown64;
	shared_ptr<FTexture> Bloomup32;
	shared_ptr<FTexture> Bloomup16;
	shared_ptr<FTexture> Bloomup8;
	shared_ptr<FTexture> Bloomup4;
};

class FFrameResourceManager
{
public:
	std::vector<FFrameResource> FrameResources;
public:
	void InitFrameResource(uint32 FrameCount);
	void CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(FScene* Scene, const uint32& FrameIndex);
	void CreateMeshActorFrameResources(FMeshActorFrameRes& MeshActorFrameResource, const FMeshActor& MeshActor);
};