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
	
	const uint32 ShadowMapSize = 8192;

	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FSampler> ClampSampler;

	shared_ptr<FMesh> PastProcessTriangle;
	shared_ptr<FMeshRes> PastProcessTriangleRes;

	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> BloomDownMap8;
	shared_ptr<FTexture> BloomDownMap16;
	shared_ptr<FTexture> BloomDownMap32;
	shared_ptr<FTexture> BloomDownMap64;
	shared_ptr<FTexture> BloomUpMap32;
	shared_ptr<FTexture> BloomUpMap16;
	shared_ptr<FTexture> BloomUpMap8;
	shared_ptr<FTexture> BloomUpMap4;

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