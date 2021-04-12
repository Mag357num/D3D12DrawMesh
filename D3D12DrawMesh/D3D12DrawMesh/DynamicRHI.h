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
		virtual void RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight) = 0; // factory, device, command, swapchain,

		// pso
		virtual void InitPipeLineToMeshRes(FShader* VS, FShader* PS, SHADER_FLAGS rootFlags, FPSOInitializer* PsoInitializer, FMeshRes* MeshRes) = 0;

		// mesh
		virtual shared_ptr<FMesh> PrepareMeshData(const std::string& BinFileName) = 0;
		virtual void UpLoadMesh(FMesh* Mesh) = 0;

		// mesh res
		virtual shared_ptr<FShader> CreateVertexShader(LPCWSTR FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(LPCWSTR FileName) = 0;
		virtual shared_ptr<FMeshRes> CreateMeshRes(std::wstring FileName, SHADER_FLAGS flags) = 0;
		virtual void CreateConstantBufferToMeshRes(FMeshRes* MeshRes) = 0;
		virtual void UpdateConstantBufferInMeshRes(FMeshRes* MeshRes, FCBData* Data) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void DrawScene(FScene Scene) = 0;
		virtual void DrawActor(FActor* Actor) = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void SyncFrame() = 0;

	};
}
