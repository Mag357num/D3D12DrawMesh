#pragma once
#include "stdafx.h"
#include "DynamicRHI.h"
#include "DX12Resource.h"


namespace RHI
{
	struct FCommand
	{
		FCommand(ComPtr<ID3D12CommandQueue> CQ) { CommandQueue = CQ; }

		ComPtr<ID3D12CommandAllocator> Allocator;
		ComPtr<ID3D12GraphicsCommandList> CommandList;
		ComPtr<ID3D12CommandQueue> CommandQueue;

		void Create(ComPtr<ID3D12Device> Device);
		void Close();
		void Execute();
		void Reset();
	};

	class FDX12DynamicRHI : public FDynamicRHI
	{
	public:
		FDX12DynamicRHI();
		~FDX12DynamicRHI() = default;

		// init
		virtual void RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth,
			const uint32& ResoHeight) override;

		// Resource Create
		virtual FMeshActor CreateMeshActor(uint32 VertexStride, vector<float> Vertices, vector<uint32> Indices, FTransform Transform) override;
		virtual shared_ptr<FMesh> CreateMesh(const FMeshActor& MeshActor) override;
		virtual shared_ptr<FMeshRes> CreateMeshRes() override;
		virtual shared_ptr<FMaterial> CreateMaterial(const std::wstring& ShaderFileName, uint32 ConstantBufferSize, vector<shared_ptr<FHandle>> TexHandles, FPassType Type) override;
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) override;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) override;
		virtual shared_ptr<FCB> CreateConstantBuffer(const uint32& Size) override;
		virtual shared_ptr<FTexture> CreateTexture(FTextureType Type, uint32 Width, uint32 Height) override;
		virtual shared_ptr<FSampler> CreateAndCommitSampler(FSamplerType Type) override;
		virtual shared_ptr<FRootSignatrue> CreateRootSignatrue(FPassType Type) override;
		virtual void CreateMeshForFrameResource(FMeshActorFrameRes& MeshActorFrameResource, const FMeshActor& MeshActor) override;

		// Resource process
		virtual void UpdateConstantBuffer(FMaterial* Mat, FCBData* Data) override;
		virtual void TransitTextureState(FTexture* Tex, FRESOURCE_STATES From, FRESOURCE_STATES To) override;
		virtual void CommitTextureAsView(FTexture* Tex, FResViewType Type) override;
		virtual void ClearRenderTarget(FHandle* Handle) override;
		virtual void ClearDepthStencil(FTexture* Tex) override;
		virtual void DrawMesh(FMesh* Mesh) override;

		// Transform, Shader
		virtual void SetViewport(float Left, float Right, float Width, float Height, float MinDepth = 0.f, float MaxDepth = 1.f) override;

		// Pipeline
		virtual shared_ptr<FPipelineState> CreatePso(FPassType Type, FShader* VS, FShader* PS, FRootSignatrue* Sig) override;
		virtual void ChoosePipelineState(FPipelineState* Pso) override;

		// shader
		virtual void SetShaderInput(FPassType Type, FMaterial* Mat, FFrameResource* FrameRes) override;

		// Output Merger
		virtual void SetScissor(uint32 Left, uint32 Top, uint32 Right, uint32 Bottom) override;
		virtual void SetRenderTarget(uint32 DescriptorNum, FHandle* RtHandle, FHandle* DsHandle) override;

		// other
		virtual uint32 GetBackBufferIndex() override { return RHISwapChain->GetCurrentBackBufferIndex(); }
		virtual FHandle* GetBackBufferHandle() { return BackBuffers[GetBackBufferIndex()]->RtvHandle.get(); }

		// draw
		virtual void FrameBegin() override;
		virtual void FrameEnd() override;

		// sync
		virtual void CreateFenceAndEvent() override;
		virtual uint32 GetFrameCount() override { return FrameCount; }
		virtual uint32 GetFramIndex() override { return FrameIndex; }
		virtual void BegineCreateResource() override;
		virtual void EndCreateResource() override;

	private:
		void WaitForExecuteComplete();
		void CreateDescriptorHeaps(const uint32& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps);
		void CreateRtvToHeaps(ID3D12Resource* RtResource, FHandle* Handle);
		void CreateCbvToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, FDX12CB* FDX12CB);
		void CreateSrvToHeaps(ID3D12Resource* ShaderResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SrvDesc, FHandle* Handle);
		void CreateDsvToHeaps(ID3D12Resource* DsResource, const D3D12_DEPTH_STENCIL_VIEW_DESC& DsvDesc, FHandle* Handle);

		void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version);
		uint32 GetEnableShaderDebugFlags();
		D3D12_RASTERIZER_DESC CreateRasterizerStateDesc();
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
		void DX12CreateConstantBuffer(FDX12CB* FDX12CB, uint32 Size);
		ComPtr<ID3D12RootSignature> CreateDX12RootSig(FPassType Type);
		void CreateGPUFence(ComPtr<ID3D12Fence>& Fence);
		void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter = false);

	private:
		// RHI attributes
		ComPtr<ID3D12Device> Device;
		ComPtr<IDXGIFactory4> DxgiFactory;
		ComPtr<IDXGIFactory4> Factory;
		ComPtr<IDXGISwapChain3> RHISwapChain;
		ComPtr<ID3D12CommandQueue> RHICommandQueue;
		D3D12_VIEWPORT Viewport;
		D3D12_RECT ScissorRect;
		ComPtr<ID3D12DescriptorHeap> RTVHeap;
		ComPtr<ID3D12DescriptorHeap> DSVHeap;
		ComPtr<ID3D12DescriptorHeap> CBVSRVHeap;
		ComPtr<ID3D12DescriptorHeap> SamplerHeap;
		uint32 BackFrameIndex;
		std::vector<FCommand> CommandLists;

		//sync
		HANDLE FenceEvent;
		int FenceValue;
		ComPtr<ID3D12Fence> Fence;

		static const uint32 FrameCount = 1;
		uint32 FrameIndex = 0; // TODO: only have one Frame
		CD3DX12_CPU_DESCRIPTOR_HANDLE LastCpuHandleRt;
		CD3DX12_CPU_DESCRIPTOR_HANDLE LastCpuHandleDs;
		CD3DX12_CPU_DESCRIPTOR_HANDLE LastCpuHandleCbSr;
		CD3DX12_GPU_DESCRIPTOR_HANDLE LastGpuHandleCbSr;
		CD3DX12_CPU_DESCRIPTOR_HANDLE LastCpuHandleSampler;
		CD3DX12_GPU_DESCRIPTOR_HANDLE LastGpuHandleSampler;

		shared_ptr<FTexture> BackBuffers[RHI::BACKBUFFER_NUM];
	};
}
