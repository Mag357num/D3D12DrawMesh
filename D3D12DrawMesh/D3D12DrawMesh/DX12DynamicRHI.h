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

		//ComPtr<ID3D12Fence> Fence; // TODO: thread sync?

		void Create(ComPtr<ID3D12Device> Device);
		void Close();
		void Execute();
		void Reset();
	};

	struct FDX12PSOInitializer : public FPSOInitializer
	{
	public:
		FDX12PSOInitializer();
		FDX12PSOInitializer(const D3D12_INPUT_LAYOUT_DESC& VertexDescription, /*ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS,*/
			const D3D12_RASTERIZER_DESC& rasterizerStateDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);

		void InitPsoInitializer() override {}

		struct DXGI_SAMPLE_DESC
		{
			uint32 Count;
			uint32 Quality;
		};

		D3D12_INPUT_LAYOUT_DESC InputLayout;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_BLEND_DESC BlendState;
		D3D12_DEPTH_STENCIL_DESC DepthStencilState;
		uint32 SampleMask;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		uint32 NumRenderTargets;
		DXGI_FORMAT RTVFormats[8];
		DXGI_FORMAT DSVFormat;
		DXGI_SAMPLE_DESC SampleDesc;
	};

	class FDX12DynamicRHI : public FDynamicRHI
	{
	public:
		FDX12DynamicRHI();
		~FDX12DynamicRHI() = default;

		// init
		virtual void RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth,
			const uint32& ResoHeight) override;

		// pso
		virtual void InitPipeLineToMeshRes(FMeshRes* MeshRes, FPSOInitializer* PsoInitializer, const SHADER_FLAGS& rootFlags) override;
		

		// mesh
		virtual void CreateMeshForFrameResource(FMeshActorFrameResource& MeshActorFrameResource, FMeshActor& MeshActor) override;

		// mesh res
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) override;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) override;
		//virtual void CreateMeshResObj(FMeshRes* MeshRes, const std::wstring& FileName, const SHADER_FLAGS& flags) override;
		virtual shared_ptr<FCB> CreateConstantBufferToMeshRes(const uint32& Size, uint32 ResIndex) override;
		virtual void UpdateConstantBufferInMeshRes(FMeshRes* MeshRes, FCBData* Data) override;

		// draw
		virtual void FrameBegin() override;
		virtual void DrawFrame(const FFrameResource* FrameRes) override;
		virtual void DrawMeshActor(const FMeshActorFrameResource& MeshActor) override;
		virtual void FrameEnd() override;

		// sync
		virtual void CreateFenceAndEvent() override;
		virtual uint32 GetFrameCount() override { return FrameCount; }
		virtual uint32 GetFramIndex() override { return FrameIndex; }
		virtual void BegineCreateResource() override;
		virtual void EndCreateResource() override;

	private:
		shared_ptr<FMesh> CommitMeshBuffer(FMeshActor& MeshActor);
		shared_ptr<FMeshRes> CommitMeshResBuffer(const std::wstring& FileName, const SHADER_FLAGS& flags, uint32 ResIndex);

		inline void GetBackBufferIndex() { BackFrameIndex = RHISwapChain->GetCurrentBackBufferIndex(); }
		void WaitForExecuteComplete();
		void CreateDescriptorHeaps(const uint32& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type,
			const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps);
		void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const uint32& FrameCount);
		void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap, uint32 ResIndex);
		void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, uint32 Width, uint32 Height);
		void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version);
		uint32 GetEnableShaderDebugFlags();
		D3D12_RASTERIZER_DESC CreateRasterizerStateDesc();
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
		D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const FDX12PSOInitializer& Initializer,
			ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS);
		ComPtr<ID3D12PipelineState> CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc);
		void DX12CreateConstantBuffer(FDX12CB* FDX12CB, uint32 Size, ComPtr<ID3D12DescriptorHeap>& Heap, uint32 ResIndex);
		ComPtr<ID3D12RootSignature> CreateDX12RootSig_1CB_VS();
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
		ComPtr<ID3D12Resource> RenderTargets[3]; // TODO: hard coding to 3
		ComPtr<ID3D12DescriptorHeap> RTVHeap;
		ComPtr<ID3D12DescriptorHeap> DSVHeap;
		ComPtr<ID3D12DescriptorHeap> CBVSRVHeap;
		uint32 BackFrameIndex;
		uint32 ResoWidth;
		uint32 ResoHeight;

		HANDLE FenceEvent;
		int FenceValue;
		ComPtr<ID3D12Fence> Fence;

		// may changes attributes
		ComPtr<ID3D12Resource> DepthStencil;
		std::vector<FCommand> CommandLists;

		static const uint32 FrameCount = 1;
		uint32 FrameIndex = 0; // TODO: only have one Frame
	};
}
