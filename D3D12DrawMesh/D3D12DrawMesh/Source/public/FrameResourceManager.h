#pragma once
#include "RHIResource.h"
#include "Scene.h"
#include "stdafx.h"

using namespace RHI;

struct FSingleBufferFrameResource
{
	// meshes
	shared_ptr<FGeometry> CharacterMesh;
	vector<shared_ptr<FGeometry>> StaticMeshes;
	shared_ptr<FGeometry> PPTriangle; // post process triangle

	// render resources
	unordered_map<FGeometry*, shared_ptr<FRenderResource>> RRMap_ShadowPass;
	unordered_map<FGeometry*, shared_ptr<FRenderResource>> RRMap_ScenePass;
	shared_ptr<FRenderResource> RR_BloomSetup;
	shared_ptr<FRenderResource> RR_BloomDown[4];
	shared_ptr<FRenderResource> RR_BloomUp[3];
	shared_ptr<FRenderResource> RR_SunMerge;
	shared_ptr<FRenderResource> RR_ToneMapping;

	// static light
	shared_ptr<FCB> StaticSkyLightCB;

	// sampler
	shared_ptr<FSampler> ClampSampler;
	shared_ptr<FSampler> WarpSampler;
};

struct FMultiBufferFrameResource
{
	const uint32 ShadowMapSize = 8192;

	// constant buffer
	shared_ptr<FCB> CameraCB;
	shared_ptr<FCB> CharacterPaletteCB;

	// textures // TODO: move to single buffer frame resource
	shared_ptr<FTexture> ShadowMap;
	shared_ptr<FTexture> DepthStencilMap;
	shared_ptr<FTexture> SceneColorMap;
	shared_ptr<FTexture> BloomSetupMap;
	shared_ptr<FTexture> SunMergeMap;
	vector<shared_ptr<FTexture>> BloomDownMapArray;
	vector<shared_ptr<FTexture>> BloomUpMapArray;

};

class FFrameResourceManager
{
private:
	FSingleBufferFrameResource SFrameRes;
	vector<FMultiBufferFrameResource> MFrameRes;
public:
	FSingleBufferFrameResource& GetSingleFrameRes() { return SFrameRes; }
	vector<FMultiBufferFrameResource>& GetMultiFrameRes() { return MFrameRes; }
	void InitFrameResource(TScene* Scene, const uint32& FrameCount);
	void CreateFrameResourcesFromScene(const shared_ptr<TScene> Scene, const uint32& FrameCount);
	void UpdateFrameResources(TScene* Scene, const uint32& FrameIndex);
	void UpdatePalette(vector<FMatrix> Palette, const uint32& FrameIndex);
	//void UpdateTest(int test, const uint32& FrameIndex);

	shared_ptr<FRenderResource> CreateRenderResource(const wstring& Shader, const uint32& Size, FVertexInputLayer VIL, FShaderInputLayer SIL, FFormat RtFormat, uint32 RtNum, uint32 FrameCount);

	void InitCameraConstantBuffer(TScene* Scene, FMultiBufferFrameResource& FrameRes);
	void InitLightConstantBuffer(TScene* Scene, FSingleBufferFrameResource& FrameRes);
	void InitCharacterPaletteConstantBuffer(TScene* Scene, FMultiBufferFrameResource& FrameRes);
	void CreateMapsForShadow(FMultiBufferFrameResource& FrameRes);
	void CreateSamplers();
	void CreateMapsForScene(FMultiBufferFrameResource& FrameRes);
	void CreateMapsForPostProcess(FMultiBufferFrameResource& FrameRes);
	void CreatePPTriangle();
	void CreatePPTriangleRR();
};