#pragma once
#include "stdafx.h"

namespace RHI
{
	using namespace DirectX;

	class FDynamicRHI;

	/** A global pointer to the dynamically bound RHI implementation. */
	extern FDynamicRHI* GDynamicRHI;

	struct FConstantBufferBase{};

	struct FShader {};

	struct FMesh
	{
		virtual void Init() {};

		FMesh() : PVertData(nullptr), PIndtData(nullptr), VertexBufferSize(0), VertexStride(0), IndexBufferSize(0), IndexNum(0){}
		~FMesh() { free(PVertData); free(PIndtData);} // TODO: free nullptr will clapse

		UINT8* PVertData;
		UINT8* PIndtData;
		int VertexBufferSize;
		int VertexStride;
		int IndexBufferSize;
		int IndexNum;
	};

	struct FRHIPSOInitializer
	{
		virtual void InitPsoInitializer(/*FInputLayout InputLayout, FRHIShader Shader*/) = 0;
	};

	enum
	{
		BUFFRING_NUM = 2,
		MAX_HEAP_SRV_CBV = 4096,
		MAX_HEAP_SAMPLERS = 16,
		MAX_HEAP_RENDERTARGETS = 128,
		MAX_HEAP_DEPTHSTENCILS = 32,
	};

	class FDynamicRHI
	{
	public:
		FDynamicRHI() = default;
		~FDynamicRHI() = default;

		static FDynamicRHI* DRHI;

		/* new recognize */
		static void CreateRHI();
		virtual void RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight) = 0; // factory, device, command, swapchain,
		virtual void GetBackBufferIndex() = 0;

		//update resource
		virtual void UpLoadConstantBuffer(const UINT& CBSize, const FConstantBufferBase& CBData, UINT8*& PCbvDataBegin) = 0; // Up Load ConstantBufferView To Heap

		// pipeline
		virtual void InitPipeLine() = 0;

		// mesh
		virtual void CreateVertexShader(LPCWSTR FileName) = 0;
		virtual void CreatePixelShader(LPCWSTR FileName) = 0;
		virtual FMesh* CreateMesh(const std::string& BinFileName) = 0;
		virtual void UpLoadMesh(FMesh* Mesh) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void DrawMesh(FMesh* MeshPtr) = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void SyncFrame() = 0;
	public:

	protected:
		FRHIPSOInitializer* PsoInitializer;
	};
}
