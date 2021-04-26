#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "FScene.h"
#include "FrameResourceManager.h"

namespace RHI
{
	class FDynamicRHI;
	extern shared_ptr<FDynamicRHI> GDynamicRHI;

	enum
	{
		//BUFFRING_NUM = 2,
		MAX_HEAP_SRV_CBV = 4096,
		MAX_HEAP_SAMPLERS = 16,
		MAX_HEAP_RENDERTARGETS = 128,
		MAX_HEAP_DEPTHSTENCILS = 32,
	};

	enum class FSamplerType
	{
		CLAMP = 0,
		WARP = 1,
		MIRROR = 2,
	};

	enum class SHADER_FLAGS
	{
		CB0_SR1_Sa2 = 1,
	};

	class FDynamicRHI
	{
	public:
		FDynamicRHI() = default;
		~FDynamicRHI() = default;

		static FDynamicRHI* DRHI; //TODO: remove

		// init
		static void CreateRHI();
		virtual void RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth,
			const uint32& ResoHeight) = 0;

		// Input Assembler
		virtual void SetMeshBuffer(FMesh* Mesh) = 0;

		// Resource Create
		virtual shared_ptr<FMesh> CreateMeshBuffer(const FMeshActor& MeshActor) = 0;
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FCB> CreateConstantBuffer(const uint32& Size) = 0;
		virtual shared_ptr<FTexture> CreateTexture() = 0;
		virtual shared_ptr<FHandle> CreateSRV(FTexture* Texture) = 0;

		// Resource process
		virtual void UpdateConstantBuffer(FMeshRes* MeshRes, FCBData* BaseData, FCBData* ShadowData) = 0;

		// Transform, Shader
		virtual void SetViewport(float Left, float Right, float Width, float Height, float MinDepth = 0.f, float MaxDepth = 1.f) = 0;
		virtual void SetShaderSignature(FMeshRes* MeshRes, FTexture* Texture) = 0;

		// Rasterizer
		virtual shared_ptr<FRasterizer> CreateRasterizer(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc) = 0;
		virtual void SetRasterizer(FRasterizer* Ras) = 0;

		// Output Merger
		virtual void SetScissor(uint32 Left, uint32 Top, uint32 Right, uint32 Bottom) = 0;





		// pso
		virtual void InitPipeLineToMeshRes(FMeshRes* MeshRes, FRACreater* PsoInitializer, const SHADER_FLAGS& rootFlags) = 0;

		// mesh
		virtual void CreateMeshForFrameResource(FMeshActorFrameResource& MeshActorFrameResource, const FMeshActor& MeshActor) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void DrawFrame(const FFrameResource* FrameRes) = 0;
		virtual void DrawMeshActorShadowPass(const FMeshActorFrameResource& MeshActor) = 0;
		virtual void DrawMeshActorBasePass(const FMeshActorFrameResource& MeshActor) = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void CreateFenceAndEvent() = 0;
		virtual uint32 GetFrameCount() = 0;
		virtual uint32 GetFramIndex() = 0;
		virtual void BegineCreateResource() = 0;
		virtual void EndCreateResource() = 0;


	};
}
