#pragma once
#include "RHIResource.h"
#include "Scene.h"
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

	// postprocess static mesh
	shared_ptr<FMesh> PostProcessTriangle;
	shared_ptr<FMeshRes> PostProcessTriangleRes;

	// mesh
	vector<FFrameMesh> StaticMeshArray;
	FFrameMesh SkeletalMesh;

	// constant buffer
	shared_ptr<FCB> CameraCB;
	shared_ptr<FCB> LightCB;
	shared_ptr<FCB> PaletteCB;

	// textures
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FSampler> ClampSampler;
	shared_ptr<FSampler> WarpSampler;
	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> SunMergeMap;
	vector<shared_ptr<FTexture>> BloomDownMapArray;
	vector<shared_ptr<FTexture>> BloomUpMapArray;

public:
	// pipeline
	shared_ptr<FPipeline> BloomSetupPipeline;
	shared_ptr<FPipeline> BloomDownPipeline;
	shared_ptr<FPipeline> BloomUpPipeline;
	shared_ptr<FPipeline> SunMergePipeline;
	shared_ptr<FPipeline> ToneMappingPipeline;

	// material
	shared_ptr<FMaterial> BloomSetupMat;
	shared_ptr<FMaterial> BloomDownMat[4];
	shared_ptr<FMaterial> BloomUpMat[3];
	shared_ptr<FMaterial> SunMergeMat;
	shared_ptr<FMaterial> ToneMappingMat;

public:
	const uint32& GetShadowMapSize() const { return ShadowMapSize; }
	void SetPostProcessTriangle(shared_ptr<FMesh>& Mesh) { PostProcessTriangle = Mesh; }
	void SetPostProcessTriangleRes(shared_ptr<FMeshRes>& MeshRes) { PostProcessTriangleRes = MeshRes; }
	const shared_ptr<FMesh>& GetPostProcessTriangle() const { return PostProcessTriangle; }
	const shared_ptr<FMeshRes>& GetPostProcessTriangleRes() const { return PostProcessTriangleRes; }
	vector<FFrameMesh>& GetStaticMeshArray() { return StaticMeshArray; }
	FFrameMesh& GetSkeletalMesh() { return SkeletalMesh; }

	void SetCameraCB(shared_ptr<FCB> CB) { CameraCB = CB; }
	void SetLightCB(shared_ptr<FCB> CB) { LightCB = CB; }
	void SetPaletteCB(shared_ptr<FCB> CB) { PaletteCB = CB; }
	shared_ptr<FCB> GetCameraCB() { return CameraCB; }
	shared_ptr<FCB> GetLightCB() { return LightCB; }
	shared_ptr<FCB> GetPaletteCB() { return PaletteCB; }

	void SetShadowMap(const shared_ptr<FTexture>& Tex) { ShadowMap = Tex; }
	void SetDsMap(const shared_ptr<FTexture>& Tex) { DepthStencilMap = Tex; }
	void SetSceneColorMap(const shared_ptr<FTexture>& Tex) { SceneColorMap = Tex; }
	void SetClampSampler(const shared_ptr<FSampler>& Sam) { ClampSampler = Sam; }
	void SetWarpSampler(const shared_ptr<FSampler>& Sam) { WarpSampler = Sam; }
	const shared_ptr<FTexture>& GetShadowMap() const { return ShadowMap; }
	const shared_ptr<FTexture>& GetDsMap() const { return DepthStencilMap; }
	const shared_ptr<FTexture>& GetSceneColorMap() const { return SceneColorMap; }
	const shared_ptr<FSampler>& GetClampSampler() const { return ClampSampler; }
	const shared_ptr<FSampler>& GetWarpSampler() const { return WarpSampler; }

	void SetBloomSetupMap(const shared_ptr<FTexture>& Tex) { BloomSetupMap = Tex; }
	void SetSunMergeMap(const shared_ptr<FTexture>& Tex) { SunMergeMap = Tex; }
	const shared_ptr<FTexture>& GetBloomSetupMap() const { return BloomSetupMap; }
	const shared_ptr<FTexture>& GetSunMergeMap() const { return SunMergeMap; }

	vector<shared_ptr<FTexture>>& GetBloomDownMapArray() { return BloomDownMapArray; }
	vector<shared_ptr<FTexture>>& GetBloomUpMapArray() { return BloomUpMapArray; }
};

class FFrameResourceManager
{
private:
	vector<FFrameResource> FrameResArray; // every frame own a FFrameResource
public:
	vector<FFrameResource>& GetFrameRes() { return FrameResArray; }
	void InitFrameResource(FScene* Scene, const uint32& FrameCount);
	void CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(FScene* Scene, const uint32& FrameIndex);
	FFrameMesh CreateFrameMesh(TStaticMeshComponent& MeshComponent); // TODO: combine the two CreateFrameMesh
	FFrameMesh CreateFrameMesh(TSkeletalMeshComponent& MeshComponent);
	void InitCameraConstantBuffer(FScene* Scene, FFrameResource& FrameRes);
	void InitLightConstantBuffer(FScene* Scene, FFrameResource& FrameRes);
	void InitPaletteConstantBuffer(FScene* Scene, FFrameResource& FrameRes);
	void CreateMapsForShadow(FFrameResource& FrameRes);
	void CreateSamplers(FFrameResource& FrameRes);
	void CreateMapsForScene(FFrameResource& FrameRes);
	void CreateMapsForPostProcess(FFrameResource& FrameRes);
	void CreatePostProcessTriangle(FFrameResource& FrameRes);
	void CreatePostProcessMaterials(FFrameResource& FrameRes);
	void InitPostProcessConstantBuffer(FFrameResource& FrameRes);
	void CreatePostProcessPipelines(FFrameResource& FrameRes);

	void TransitToFR(class ACamera Cam, FFrameResource& FR);
	void TransitToFR(class FLight Cam, FFrameResource& FR);

};