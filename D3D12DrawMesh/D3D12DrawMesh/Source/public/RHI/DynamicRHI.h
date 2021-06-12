#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "StaticMesh.h"
#include "SkeletalMesh.h"

namespace RHI
{
	class FDynamicRHI;
	extern shared_ptr<FDynamicRHI> GDynamicRHI;

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
		static void DestroyRHI();
		virtual void RHIInit() = 0;

		// Resource Create
		virtual shared_ptr<FGeometry> CreateGeometry(FStaticMeshComponent* MeshComponent ) = 0;
		virtual shared_ptr<FGeometry> CreateGeometry(FSkeletalMeshComponent* MeshComponent) = 0;
		virtual shared_ptr<FGeometry> CreateGeometry(FStaticMeshLOD* Lod) = 0;

		virtual shared_ptr<FShader> CreateVertexShader(const wstring& FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(const wstring& FileName) = 0;
		virtual shared_ptr<FCB> CreateConstantBuffer(const uint32& Size) = 0;
		virtual shared_ptr<FTexture> CreateTexture(FTextureType Type, uint32 Width, uint32 Height) = 0;
		virtual shared_ptr<FSampler> CreateAndCommitSampler(FSamplerType Type) = 0;
		virtual shared_ptr<FRootSignatrue> CreateRootSignatrue(FShaderInputLayer InputLayer) = 0;

		// Resource process
		virtual void WriteConstantBuffer(FCB* CB, void* Src, size_t Size) = 0;
		virtual void WriteConstantBufferWithOffset(FCB* CB, size_t Offset, void* Src, size_t Size) = 0;
		virtual void SetTextureState(FTexture* Tex, FRESOURCE_STATES State) = 0;
		virtual void SwitchTextureState(FTexture* Tex, FRESOURCE_STATES S1, FRESOURCE_STATES S2) = 0;
		virtual void TransitTextureState(FTexture* Tex, FRESOURCE_STATES From, FRESOURCE_STATES To) = 0;
		virtual void CommitTextureAsView(FTexture* Tex, FResViewType Type) = 0;
		virtual void ClearDepthStencil(FTexture* Tex) = 0;
		virtual void ClearRenderTarget(FHandle* Handle) = 0;
		virtual void DrawGeometry(FGeometry* Mesh) = 0;

		// Transform, Shader
		virtual void SetViewport(float Left, float Right, float Width, float Height, float MinDepth = 0.f, float MaxDepth = 1.f) = 0;

		// Pipeline
		virtual shared_ptr<FPipelineState> CreatePso(FFormat RtFormat, FBlendMode BlendMode, FVertexInputLayer Layer, uint32 NumRt, FShader* VS, FShader* PS, FRootSignatrue* Sig) = 0;
		virtual void SetPipelineState(FRenderResource* RR) = 0;
		virtual void SetShaderInput(vector<shared_ptr<FHandle>> Handles) = 0;

		// Output Merger
		virtual void SetScissor(uint32 Left, uint32 Top, uint32 Right, uint32 Bottom) = 0;
		virtual void SetRenderTarget(uint32 DescriptorNum, FHandle* RtHandle, FHandle* DsHandle) = 0;

		// other
		virtual FHandle* GetBackBufferHandle() = 0;

		// draw
		virtual void FrameBegin() = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual const uint32& GetFrameCount() = 0;
		virtual const uint32& GetFramIndex() = 0;
		virtual void BegineCreateResource() = 0;
		virtual void EndCreateResource() = 0;

		//even
		virtual void BeginEvent(const char* EventName) = 0;
		virtual void EndEvent() = 0;
	};
}

class FScopedEvent
{
public:
	FScopedEvent(const char* EventName)
	{
		RHI::GDynamicRHI->BeginEvent(EventName);
	}
	~FScopedEvent() { RHI::GDynamicRHI->EndEvent(); }
};

#define MACRO_COMBINE_DIRECT(X, Y) X##Y
#define MACRO_COMBINE(X, Y) MACRO_COMBINE_DIRECT(X, Y)
#define SCOPED_EVENT(__NAME__) FScopedEvent MACRO_COMBINE(ScopedEvent, __LINE__)(__NAME__)
//#define SCOPED_EVENT_F(...) SCOPED_EVENT(fmt::format(__VA_ARGS__).c_str())
