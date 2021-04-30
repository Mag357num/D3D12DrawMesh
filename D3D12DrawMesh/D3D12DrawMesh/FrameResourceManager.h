#pragma once
#include "RHIResource.h"
#include "FScene.h"
#include "stdafx.h"

using namespace RHI;

struct FFrameMesh
{
	shared_ptr<FMesh> Mesh;
	shared_ptr<FMeshRes> MeshRes;
	vector<FTexture> Textures;
};

class FFrameResource
{
private:
	const uint32 ShadowMapSize = 8192;
	shared_ptr<FMesh> PostProcessTriangle;
	shared_ptr<FMeshRes> PostProcessTriangleRes;
	vector<FFrameMesh> FrameMeshes;

public:
	const uint32& GetShadowMapSize() const { return ShadowMapSize; }
	void SetPostProcessTriangle( shared_ptr<FMesh>& Mesh ) { PostProcessTriangle = Mesh; }
	void SetPostProcessTriangleRes( shared_ptr<FMeshRes>& MeshRes ) { PostProcessTriangleRes = MeshRes; }
	const shared_ptr<FMesh>& GetPostProcessTriangle() const { return PostProcessTriangle; }
	const shared_ptr<FMeshRes>& GetPostProcessTriangleRes() const { return PostProcessTriangleRes; }
	vector<FFrameMesh>& GetFrameMeshes() { return FrameMeshes; }

public: // TODO: encapsulate those below in array
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FSampler> ClampSampler;
	shared_ptr<FSampler> WarpSampler;
	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> BloomDownMap8;
	shared_ptr<FTexture> BloomDownMap16;
	shared_ptr<FTexture> BloomDownMap32;
	shared_ptr<FTexture> BloomDownMap64;
	shared_ptr<FTexture> BloomUpMap32;
	shared_ptr<FTexture> BloomUpMap16;
	shared_ptr<FTexture> BloomUpMap8;
	shared_ptr<FTexture> SunMergeMap;
};

class FFrameResourceManager
{
private:
	vector<FFrameResource> FrameRes;
public:
	vector<FFrameResource>& GetFrameRes() { return FrameRes; }
	void InitFrameResource(const uint32& FrameCount);
	void CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(FScene* Scene, const uint32& FrameIndex);
	void CreateMeshActorFrameResources(FFrameMesh& MeshActorFrameResource, const FMeshActor& MeshActor);
};