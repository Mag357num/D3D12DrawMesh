#pragma once
#include "stdafx.h"
#include "RHIResource.h"
#include "StaticMeshActor.h"

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
		virtual shared_ptr<FMesh> CreateMesh(const FStaticMeshActor& MeshActor) = 0; // meshActor is mesh data, can read from file or write in code
		virtual shared_ptr<FMaterial> CreateMaterial(const wstring& ShaderFileName, uint32 ConstantBufferSize, vector<shared_ptr<FHandle>> TexHandles) = 0;
		virtual shared_ptr<FShader> CreateVertexShader(const wstring& FileName) = 0;
		virtual shared_ptr<FShader> CreatePixelShader(const wstring& FileName) = 0;
		virtual shared_ptr<FCB> CreateConstantBuffer(const uint32& Size) = 0;
		virtual shared_ptr<FTexture> CreateTexture(FTextureType Type, uint32 Width, uint32 Height) = 0;
		virtual shared_ptr<FSampler> CreateAndCommitSampler(FSamplerType Type) = 0;
		virtual shared_ptr<FRootSignatrue> CreateRootSignatrue(FShaderInputLayer InputLayer) = 0;

		// Resource process
		virtual void UpdateConstantBuffer(FMaterial* Mat, FCBData* Data) = 0;
		virtual void TransitTextureState(FTexture* Tex, FRESOURCE_STATES From, FRESOURCE_STATES To) = 0;
		virtual void CommitTextureAsView(FTexture* Tex, FResViewType Type) = 0;
		virtual void ClearDepthStencil(FTexture* Tex) = 0;
		virtual void ClearRenderTarget(FHandle* Handle) = 0;
		virtual void DrawMesh(FMesh* Mesh) = 0;

		// Transform, Shader
		virtual void SetViewport(float Left, float Right, float Width, float Height, float MinDepth = 0.f, float MaxDepth = 1.f) = 0;

		// Pipeline
		virtual shared_ptr<FPipeline> CreatePipeline(FFormat RtFormat, uint32 RtNum, FVertexInputLayer VertexInputLayer, FShaderInputLayer ShaderInputLayer, FMaterial* Mat) = 0;
		virtual shared_ptr<FPipelineState> CreatePso(FFormat RtFormat, FVertexInputLayer Layer, uint32 NumRt, FShader* VS, FShader* PS, FRootSignatrue* Sig) = 0;
		virtual void SetPipelineState(FPipeline* Pipe) = 0;
		virtual void SetShaderInput(vector<shared_ptr<FHandle>> Handles) = 0;

		// Output Merger
		virtual void SetScissor(uint32 Left, uint32 Top, uint32 Right, uint32 Bottom) = 0;
		virtual void SetRenderTarget(uint32 DescriptorNum, FHandle* RtHandle, FHandle* DsHandle) = 0;

		// other
		virtual uint32 GetBackBufferIndex() = 0;
		virtual FHandle* GetBackBufferHandle() = 0;
		inline uint32 GetWidth() { return ResoWidth; }
		inline uint32 GetHeight() { return ResoHeight; }

		// draw
		virtual void FrameBegin() = 0;
		virtual void FrameEnd() = 0;

		// sync
		virtual void CreateFenceAndEvent() = 0;
		virtual uint32 GetFrameNum() = 0;
		virtual uint32 GetFramIndex() = 0;
		virtual void BegineCreateResource() = 0;
		virtual void EndCreateResource() = 0;

		//even
		virtual void BeginEvent(const char* EventName) = 0;
		virtual void EndEvent() = 0;

		uint32 ResoWidth;
		uint32 ResoHeight;
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
