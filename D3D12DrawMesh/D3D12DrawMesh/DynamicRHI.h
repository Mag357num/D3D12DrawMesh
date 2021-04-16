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

	enum class SHADER_FLAGS
	{
		CB1_SR0 = 1, // use 1 cb, 0 sr
	};

	class FDynamicRHI
	{
	public:
		FDynamicRHI() = default;
		~FDynamicRHI() = default;

		static FDynamicRHI* DRHI;

		// init
		static void CreateRHI();
		virtual void RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth,
			const uint32& ResoHeight) = 0;

		// pso
		virtual void InitPipeLineToMeshRes(FMeshRes* MeshRes, FPSOInitializer* PsoInitializer, const SHADER_FLAGS& rootFlags) = 0;

		// mesh
		virtual void CreateMeshForFrameResource(FMeshActorFrameResource& MeshActorFrameResource, FMeshActor& MeshActor) = 0;

		// mesh res
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) = 0;
		//virtual void CreateMeshResObj(FMeshRes* MeshRes, const std::wstring& FileName, const SHADER_FLAGS& flags) = 0;
		virtual shared_ptr<FCB> CreateConstantBufferToMeshRes(const uint32& Size, uint32 ResIndex) = 0;
		virtual void UpdateConstantBufferInMeshRes(FMeshRes* MeshRes, FCBData* Data) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void DrawFrame(const FFrameResource* FrameRes) = 0;
		virtual void DrawMeshActor(const FMeshActorFrameResource& MeshActor) = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void CreateFenceAndEvent() = 0;
		virtual uint32 GetFrameCount() = 0;
		virtual uint32 GetFramIndex() = 0;
		virtual void BegineCreateResource() = 0;
		virtual void EndCreateResource() = 0;
	};
}
