#pragma once
#include "stdafx.h"

namespace RHI
{
	/*
	* TODO:
	1. 使用ComPtr、share_ptr解决内存泄露
	2. *漫游系统重新搭建起来
	3. *绘制场景
	*/

	using namespace DirectX;

	class FDynamicRHI;

	extern FDynamicRHI* GDynamicRHI;

	struct FCBData 
	{
		FCBData() : BufferData(nullptr), BufferSize(256) {};
		void* BufferData;
		UINT BufferSize;
	};

	struct FShader
	{
		virtual void Init() {};
	};

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

	struct FMeshRes
	{
		FMeshRes() : WorldTrans(XMMatrixTranslation(0.f, 0.f, 0.f)), VS(nullptr), PS(nullptr) {}
		virtual void init() {};

		XMMATRIX WorldTrans;
		FShader* VS;
		FShader* PS;
	};

	struct FActor
	{
		FMesh* Mesh;
		FMeshRes* MeshRes;
	};

	struct FScene
	{
		std::vector<FActor*> Actors;
	};

	struct FRHIPSOInitializer
	{
		virtual void InitPsoInitializer(/*FInputLayout InputLayout, FRHIShader Shader*/) = 0;
	};

	struct FCB
	{
	};

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
		virtual void InitPipeLineToMeshRes(FShader* VS, FShader* PS, SHADER_FLAGS rootFlags, FRHIPSOInitializer* PsoInitializer, FMeshRes* MeshRes) = 0;

		// mesh
		virtual FMesh* CreateMesh(const std::string& BinFileName) = 0;
		virtual void UpLoadMesh(FMesh* Mesh) = 0;

		// mesh res
		virtual FShader* CreateVertexShader(LPCWSTR FileName) = 0;
		virtual FShader* CreatePixelShader(LPCWSTR FileName) = 0;
		virtual FMeshRes* CreateMeshRes(std::wstring FileName, SHADER_FLAGS flags) = 0;
		virtual void CreateConstantBufferToMeshRes(FMeshRes* MeshRes) = 0;
		virtual void UpdateConstantBufferInMeshRes(FMeshRes* MeshRes, FCBData* Data) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void DrawScene(FScene Scene) = 0;
		virtual void DrawActor(FActor* Actor) = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void SyncFrame() = 0;

		// release
		virtual void ReleActor(FActor* Actor) = 0;
	};
}
