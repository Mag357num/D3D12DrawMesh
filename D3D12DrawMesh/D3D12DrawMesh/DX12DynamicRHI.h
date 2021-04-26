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

	struct FDX12PSOInitializer : public FRACreater
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

		// Input Assembler
		virtual void SetMeshBuffer(FMesh* Mesh) override;

		// Resource Create
		virtual shared_ptr<FMesh> CreateMeshBuffer(const FMeshActor& MeshActor) override;
		virtual shared_ptr<FShader> CreateVertexShader(const std::wstring& FileName) override;
		virtual shared_ptr<FShader> CreatePixelShader(const std::wstring& FileName) override;
		virtual shared_ptr<FCB> CreateConstantBuffer(const uint32& Size) override;
		virtual shared_ptr<FTexture> CreateTexture(FResourceDesc* Desc) override;

		// Resource process
		virtual void UpdateConstantBuffer(FMeshRes* MeshRes, FCBData* BaseData, FCBData* ShadowData) override;

		// Transform, Shader
		virtual void SetViewport(float Left, float Right, float Width, float Height, float MinDepth = 0.f, float MaxDepth = 1.f) override;
		virtual void SetShaderSignature(FMeshRes* MeshRes, FTexture* Texture) override;

		// Rasterizer
		virtual shared_ptr<FRasterizer> CreateRasterizer(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc) override;
		virtual void SetRasterizer(FRasterizer* Ras) override;

		// Output Merger
		virtual void SetScissor(uint32 Left, uint32 Top, uint32 Right, uint32 Bottom) override;



		// pso
		virtual void InitPipeLineToMeshRes(FMeshRes* MeshRes, FRACreater* PsoInitializer, const SHADER_FLAGS& rootFlags) override;

		// mesh
		virtual void CreateMeshForFrameResource(FMeshActorFrameResource& MeshActorFrameResource, const FMeshActor& MeshActor) override;

		// draw
		virtual void FrameBegin() override;
		virtual void DrawFrame(const FFrameResource* FrameRes) override;
		virtual void DrawMeshActorShadowPass(const FMeshActorFrameResource& MeshActor) override;
		virtual void DrawMeshActorBasePass(const FMeshActorFrameResource& MeshActor) override;
		virtual void FrameEnd() override;

		// sync
		virtual void CreateFenceAndEvent() override;
		virtual uint32 GetFrameCount() override { return FrameCount; }
		virtual uint32 GetFramIndex() override { return FrameIndex; }
		virtual void BegineCreateResource() override;
		virtual void EndCreateResource() override;

	private:
		shared_ptr<FMesh> CommitMeshBuffer(const FMeshActor& MeshActor);
		shared_ptr<FMeshRes> CommitMeshResBuffer(const std::wstring& FileName, const SHADER_FLAGS& flags);

		inline void GetBackBufferIndex() { BackFrameIndex = RHISwapChain->GetCurrentBackBufferIndex(); }
		void WaitForExecuteComplete();
		void CreateDescriptorHeaps(const uint32& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type,
			const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps);
		void CreateRTVToHeaps(const uint32& FrameCount);
		void CreateDSVToHeaps_TODO();
		void CreateSamplerToHeaps(FSamplerType Type);
		void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, FDX12CB* FDX12CB);
		void CreateSRVToHeaps(ID3D12Resource* ShaderResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SrvDesc, CD3DX12_GPU_DESCRIPTOR_HANDLE& Handle);
		void CommitShadowMap();
		void CreateShadowMapToDSVHeaps();
		void CreateShadowMapToCBVSRVHeaps();


		void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version);
		uint32 GetEnableShaderDebugFlags();
		D3D12_RASTERIZER_DESC CreateRasterizerStateDesc();
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
		D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const FDX12PSOInitializer& Initializer,
			ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS);
		void DX12CreateConstantBuffer(FDX12CB* FDX12CB, uint32 Size);
		ComPtr<ID3D12RootSignature> CreateDX12RootSig_CB0_SR1_Sa2();
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
		ComPtr<ID3D12DescriptorHeap> SamplerHeap;
		uint32 BackFrameIndex;
		uint32 ResoWidth;
		uint32 ResoHeight;
		std::vector<FCommand> CommandLists;

		//sync
		HANDLE FenceEvent;
		int FenceValue;
		ComPtr<ID3D12Fence> Fence;

		// frame resource
		ComPtr<ID3D12Resource> DepthStencilBuffer;
		CD3DX12_CPU_DESCRIPTOR_HANDLE DSVHandle;
		ComPtr<ID3D12Resource> DX12ShadowMap;
		CD3DX12_CPU_DESCRIPTOR_HANDLE ShadowDepthViewHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE ShadowMapGPUHandleForCBVSRV;
		static const uint32 FrameCount = 1;
		uint32 FrameIndex = 0; // TODO: only have one Frame
		CD3DX12_CPU_DESCRIPTOR_HANDLE LastCPUHandleForDSV;
		CD3DX12_CPU_DESCRIPTOR_HANDLE LastCPUHandleForCBVSRV;
		CD3DX12_GPU_DESCRIPTOR_HANDLE LastGPUHandleForCBVSRV;
	};
}
