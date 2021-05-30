#pragma once
#include "RHIResource.h"
#include "Scene.h"
#include "stdafx.h"

using namespace RHI;

struct FFrameMesh_deprecated
{
	shared_ptr<FMesh_deprecated> Mesh;
	shared_ptr<FMeshRes> MeshRes;
};

struct FSingleBufferFrameResource
{
	shared_ptr<FMesh_new> CharacterMesh;
	vector<shared_ptr<FMesh_new>> StaticMeshes;
	vector<shared_ptr<FMesh_new>> PPTriangle; // post process triangle

	// static light
	shared_ptr<FCB> StaticSkyLightCB;

	// sampler
	shared_ptr<FSampler> ClampSampler;
	shared_ptr<FSampler> WarpSampler;

	// postprocess static mesh
	shared_ptr<FMesh_deprecated> PostProcessTriangle;
	shared_ptr<FMeshRes> PostProcessTriangleRes;
};

struct FMultiBufferFrameResource
{
	const uint32 ShadowMapSize = 8192;

	// render resource of mesh
	vector<shared_ptr<FRenderResource_new>> RR_ShadowPass;
	vector<shared_ptr<FRenderResource_new>> RR_ScenePass;

	// constant buffer
	shared_ptr<FCB> CameraCB;
	shared_ptr<FCB> CharacterPaletteCB;

	// textures
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> SunMergeMap;
	vector<shared_ptr<FTexture>> BloomDownMapArray;
	vector<shared_ptr<FTexture>> BloomUpMapArray;

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
};

class FFrameResourceManager
{
private:
	FSingleBufferFrameResource SingleBufferFrameRes;
	vector<FMultiBufferFrameResource> MultiBufferFrameRes;
public:
	FSingleBufferFrameResource& GetStaticFrameRes() { return SingleBufferFrameRes; }
	vector<FMultiBufferFrameResource>& GetDynamicFrameRes() { return MultiBufferFrameRes; }
	void InitFrameResource(TScene* Scene, const uint32& FrameCount);
	void CreateFrameResourcesFromScene(const shared_ptr<TScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(TScene* Scene, const uint32& FrameIndex);

	shared_ptr<FRenderResource_new> CreateRenderResource(const wstring& Shader, const uint32& Size, FVertexInputLayer VIL, FShaderInputLayer SIL, FFormat RtFormat, uint32 RtNum);

	FFrameMesh_deprecated CreateFrameMesh_deprecated(TStaticMeshComponent& MeshComponent); // TODO: combine the two CreateFrameMesh
	FFrameMesh_deprecated CreateFrameMesh_deprecated(TSkeletalMeshComponent& MeshComponent);
	void InitCameraConstantBuffer(TScene* Scene, FMultiBufferFrameResource& FrameRes);
	void InitLightConstantBuffer(TScene* Scene, FSingleBufferFrameResource& FrameRes);
	void InitCharacterPaletteConstantBuffer(TScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateMapsForShadow(FMultiBufferFrameResource& FrameRes);
	void CreateSamplers();
	void CreateMapsForScene(FMultiBufferFrameResource& FrameRes);
	void CreateMapsForPostProcess(FMultiBufferFrameResource& FrameRes);
	void CreatePostProcessTriangle();
	void CreatePostProcessMaterials(FMultiBufferFrameResource& FrameRes);
	void InitPostProcessConstantBuffer(FMultiBufferFrameResource& FrameRes);
	void CreatePostProcessPipelines(FMultiBufferFrameResource& FrameRes);
};