#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "FScene.h"

namespace RHI
{
	class FDynamicRHI;
	extern shared_ptr<FDynamicRHI> GDynamicRHI;

	enum
	{
		BUFFRING_NUM = 2,
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
		virtual void RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth, const uint32& ResoHeight) = 0;

		// pso
		virtual void InitPipeLineToMeshRes(FMeshRes* MeshRes, FPSOInitializer* PsoInitializer, const SHADER_FLAGS& rootFlags) = 0;

		// mesh
		virtual shared_ptr<FMesh> PrepareMeshData(const std::string& BinFileName) = 0;
		virtual void UpLoadMesh(FMesh* Mesh) = 0;

		// mesh res
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FMeshRes> CreateMeshRes(const std::wstring& FileName, const SHADER_FLAGS& flags) = 0;
		virtual shared_ptr<FCB> CreateConstantBufferToMeshRes(const uint32& Size) = 0;
		virtual void UpdateConstantBufferInMeshRes(FMeshRes* MeshRes, FCBData* Data) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void DrawScene(const FScene* Scene) = 0;
		virtual void DrawActor(const FActor* Actor) = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void SyncFrame() = 0;
		virtual uint32 GetFramCount() = 0;
		//virtual uint32 GetFramIndex() = 0;

	};
}
