#pragma once
#include "Scene.h"
#include "stdafx.h"
#include "RHIResource.h"
#include "Material.h"

using namespace RHI;
struct FSingleBufferFrameResource
{
	// geometrys for render
	shared_ptr<FGeometry> CharacterMesh;
	shared_ptr<FGeometry> DirectionalLightMesh;
	vector<shared_ptr<FGeometry>> PointLightMeshes;
	vector<shared_ptr<FGeometry>> StaticMeshes_Translucent;
	vector<shared_ptr<FGeometry>> StaticMeshes;
	map<float, uint32> TranslucentActorIndice; // ordered by distance
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

	// post process texture
	const uint32 ShadowMapSize = 8192;
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> SunMergeMap;
	vector<shared_ptr<FTexture>> BloomDownMapArray;
	vector<shared_ptr<FTexture>> BloomUpMapArray;

	// constant buffer that dont change
	shared_ptr<FCB> BloomSetupCB;
	array<shared_ptr<FCB>, 4> BloomDownCBs;
	array<shared_ptr<FCB>, 3> BloomUpCBs;
	shared_ptr<FCB> SunMergeCB;
	unordered_map<FGeometry*, shared_ptr<FCB>> MaterialCBs; // one geo one cb

	// material texture
	unordered_map<FGeometry*, vector<shared_ptr<FTexture>>> MaterialTexs; // one geo many tex
};

struct FMultiBufferFrameResource
{
	// changeable constant buffer
	shared_ptr<FCB> CameraCB;
	shared_ptr<FCB> CharacterPaletteCB;
	shared_ptr<FCB> DirectionalLight_LightingInfoCB;
	shared_ptr<FCB> PointLights_LightingInfoCB;
	vector<shared_ptr<FCB>> DirectionalLight_LocatingCBs;
	vector<shared_ptr<FCB>> PointLight_LocatingCBs;
	shared_ptr<FCB> Character_ShadowPass_LocatingCB;
	shared_ptr<FCB> Character_ScenePass_LocatingCB;
	vector<shared_ptr<FCB>> StaticMesh_ShadowPass_LocatingCBs;
	vector<shared_ptr<FCB>> StaticMesh_ScenePass_LocatingCBs;
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

	shared_ptr<FRenderResource> CreateRenderResource(const wstring& Shader, EBlendMode BlendMode, FVertexInputLayer VIL, FShaderInputLayer SIL, FFormat RtFormat, uint32 RtNum, uint32 FrameCount);

	// create common shared cb
	void CreateCameraCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateCharacterPaletteCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateDirectionalLights_LightingInfoCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreatePointLights_LightingInfoCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateDirectionalLight_LocatingCBs(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreatePointLight_LocatingCBs(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateStaticMesh_LocatingCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateCharacter_LocatingCB(FScene* Scene, FMultiBufferFrameResource& FrameRes);

	// create common shared texture
	void CreateMapsForShadow();
	void CreateSamplers();
	void CreateMapsForScene();
	void CreateMapsForPostProcess();
	void CreatePostProcessCB();

	// post process triangle
	void CreatePPTriangle();
	void CreatePPTriangleRR();
};