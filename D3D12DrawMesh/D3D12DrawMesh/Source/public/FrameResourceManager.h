#pragma once
#include "Scene.h"
#include "stdafx.h"
#include "RHIResource.h"

using namespace RHI;
struct FSingleBufferFrameResource
{
	// geometrys for render
	shared_ptr<FGeometry> CharacterMesh;
	shared_ptr<FGeometry> DirectionalLightMesh;
	vector<shared_ptr<FGeometry>> PointLightMeshes;
	vector<shared_ptr<FGeometry>> StaticMeshes;
	shared_ptr<FGeometry> PPTriangle; // post process triangle

	// static actor's constant buffer
	
	// render resources for actors
	unordered_map<FGeometry*, shared_ptr<FRenderResource>> RRMap_ShadowPass;
	unordered_map<FGeometry*, shared_ptr<FRenderResource>> RRMap_ScenePass;

	// render resources for post process
	shared_ptr<FRenderResource> RR_BloomSetup;
	shared_ptr<FRenderResource> RR_BloomDown[4];
	shared_ptr<FRenderResource> RR_BloomUp[3];
	shared_ptr<FRenderResource> RR_SunMerge;
	shared_ptr<FRenderResource> RR_ToneMapping;

	// common shared sampler
	shared_ptr<FSampler> ClampSampler;
	shared_ptr<FSampler> WarpSampler;

	// common shared textures
	const uint32 ShadowMapSize = 8192;
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> SunMergeMap;
	vector<shared_ptr<FTexture>> BloomDownMapArray;
	vector<shared_ptr<FTexture>> BloomUpMapArray;
};

struct FMultiBufferFrameResource
{
	// changeable constant buffer
	shared_ptr<FCB> CameraCB;
	shared_ptr<FCB> CharacterPaletteCB;
	shared_ptr<FCB> DirectionalLightCB;
	shared_ptr<FCB> PointLightsCB;
};

class FFrameResourceManager
{
private:
	FSingleBufferFrameResource SFrameRes;
	vector<FMultiBufferFrameResource> MFrameRes;

public:
	FSingleBufferFrameResource& GetSingleFrameRes() { return SFrameRes; }
	vector<FMultiBufferFrameResource>& GetMultiFrameRes() { return MFrameRes; }

	void InitFrameResource(FScene* Scene, const uint32& FrameCount);
	void CreateActorsFrameRes(const shared_ptr<FScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(FScene* Scene, const uint32& FrameIndex);

	shared_ptr<FRenderResource> CreateRenderResource(const wstring& Shader, const uint32& Size, FVertexInputLayer VIL, FShaderInputLayer SIL, FFormat RtFormat, uint32 RtNum, uint32 FrameCount);

	// create common shared cb
	void CreateCameraCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateCharacterPaletteCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateDirectionalLightCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreatePointLightsCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);

	// create common shared texture
	void CreateMapsForShadow();
	void CreateSamplers();
	void CreateMapsForScene();
	void CreateMapsForPostProcess();

	// post process triangle
	void CreatePPTriangle();
	void CreatePPTriangleRR();
};