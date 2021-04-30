#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "FScene.h"
#include "FrameResourceManager.h"

namespace RHI
{
	class FDynamicRHI;
	extern shared_ptr<FDynamicRHI> GDynamicRHI;
	extern constexpr uint32 BACKBUFFER_NUM = 2;

	enum
	{
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

		// init
		static void CreateRHI();
		virtual void RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth,
			const uint32& ResoHeight) = 0;

		// Resource Create
		virtual FMeshActor CreateMeshActor(uint32 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform) = 0;
		virtual shared_ptr<FMesh> CreateMesh(const FMeshActor& MeshActor) = 0; // meshActor is mesh data, can read from file or write in code
		//virtual shared_ptr<FMeshRes> CreateMeshRes(const std::wstring& ShaderFileName, uint32 ConstantBufferSize, FMeshType_deprecated Type) = 0;
		virtual shared_ptr<FMeshRes> CreateMeshRes() = 0;
		virtual shared_ptr<FMaterial> CreateMaterial(const std::wstring& ShaderFileName, uint32 ConstantBufferSize, FPassType Type) = 0;
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) = 0;
		virtual shared_ptr<FCB> CreateConstantBuffer(const uint32& Size) = 0;
		virtual shared_ptr<FTexture> CreateTexture(FTextureType Type, uint32 Width, uint32 Height) = 0;
		virtual shared_ptr<FSampler> CreateAndCommitSampler(FSamplerType Type) = 0;
		virtual shared_ptr<FRootSignatrue> CreateRootSignatrue(FPassType Type) = 0;

		// Resource process
		virtual void UpdateConstantBuffer(FMeshRes* MeshRes, FCBData* BaseData, FCBData* ShadowData) = 0;
		virtual void TransitTextureState(FTexture* Tex, FRESOURCE_STATES From, FRESOURCE_STATES To) = 0;
		virtual void CommitTextureAsView(FTexture* Tex, FResViewType Type) = 0;
		virtual void ClearDepthStencil(FTexture* Tex) = 0;
		virtual void ClearRenderTarget(FHandle* Handle) = 0;
		virtual void DrawMesh(FMesh* Mesh) = 0;

		// Transform, Shader
		virtual void SetViewport(float Left, float Right, float Width, float Height, float MinDepth = 0.f, float MaxDepth = 1.f) = 0;

		// shader
		virtual void SetShaderInput(FPassType Type, FMeshRes* MeshRes, FFrameResource* FrameRes) = 0;

		// Pipeline
		virtual shared_ptr<FPipelineState> CreatePso(FPassType Type, FShader* VS, FShader* PS, FRootSignatrue* Sig) = 0;
		virtual void ChoosePipelineState(FPipelineState* Pso) = 0;

		// Output Merger
		virtual void SetScissor(uint32 Left, uint32 Top, uint32 Right, uint32 Bottom) = 0;
		virtual void SetRenderTarget(uint32 DescriptorNum, FHandle* RtHandle, FHandle* DsHandle) = 0;

		// other
		virtual uint32 GetBackBufferIndex() = 0;
		virtual FHandle* GetBackBufferHandle() = 0;
		inline uint32 GetWidth() { return ResoWidth; }
		inline uint32 GetHeight() { return ResoHeight; }



		// mesh
		virtual void CreateMeshForFrameResource(FMeshActorFrameRes& MeshActorFrameResource, const FMeshActor& MeshActor) = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void CreateFenceAndEvent() = 0;
		virtual uint32 GetFrameCount() = 0;
		virtual uint32 GetFramIndex() = 0;
		virtual void BegineCreateResource() = 0;
		virtual void EndCreateResource() = 0;

	protected:
		uint32 ResoWidth;
		uint32 ResoHeight;
	};
}
