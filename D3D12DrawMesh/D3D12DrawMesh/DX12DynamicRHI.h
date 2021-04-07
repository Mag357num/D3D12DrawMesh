#pragma once
#include "stdafx.h"
#include "DynamicRHI.h"

namespace RHI
{
	using namespace DirectX;

	struct FDX12Shader : public FShader
	{
		ComPtr<ID3DBlob> Shader;
	};

	struct FCBData
	{
		XMFLOAT4X4 worldViewProj;
		float padding[48]; // Padding so the constant buffer is 256-byte aligned.
	};
	static_assert((sizeof(FCBData) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	struct FDX12CB : public FCB
	{
		ComPtr<ID3D12Resource> CBObj;
		FCBData CBData;
		UINT8* PDataBegin;
	};

	struct FDX12MeshRes : public FMeshRes
	{
		ComPtr<ID3D12PipelineState> PSObj;
		ComPtr<ID3D12RootSignature> RootSignature;
		FDX12CB CB;
	};

	struct FDX12Mesh : public FMesh
	{
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		ComPtr<ID3D12Resource> VertexBufferUploadHeap;
		ComPtr<ID3D12Resource> IndexBufferUploadHeap;
	};

	struct FCommandListDx12
	{
		ComPtr<ID3D12CommandAllocator> Allocators[BUFFRING_NUM]; // TODO: per commandlist with BUFFRING_NUM allocators, why?
		ComPtr<ID3D12GraphicsCommandList> CommandList;
		//ComPtr<ID3D12Fence> Fence; // TODO: thread sync?

		void Create(ComPtr<ID3D12Device> Device);
		void Reset();
	};

	struct FDX12PSOInitializer : public FRHIPSOInitializer
	{
	public:
		FDX12PSOInitializer();
		FDX12PSOInitializer(const D3D12_INPUT_LAYOUT_DESC& VertexDescription, /*ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS,*/
			const D3D12_RASTERIZER_DESC& rasterizerStateDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);

		void InitPsoInitializer() override {}

		struct DXGI_SAMPLE_DESC
		{
			UINT Count;
			UINT Quality;
		};

		D3D12_INPUT_LAYOUT_DESC InputLayout;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_BLEND_DESC BlendState;
		D3D12_DEPTH_STENCIL_DESC DepthStencilState;
		UINT SampleMask;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		UINT NumRenderTargets;
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
		virtual void RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight) override;
		std::vector<FCommandListDx12> GraphicsCommandLists;

		// pso
		virtual void InitPipeLineToMeshRes(FShader* VS, FShader* PS, SHADER_FLAGS rootFlags, FRHIPSOInitializer* PsoInitializer, FMeshRes* MeshRes) override;
		
		// mesh
		FMesh* CreateMesh(const std::string& BinFileName) override;
		virtual void UpLoadMesh(FMesh* Mesh) override;

		// mesh res
		virtual FShader* CreateVertexShader(LPCWSTR FileName) override;
		virtual FShader* CreatePixelShader(LPCWSTR FileName) override;
		virtual FMeshRes* CreateMeshRes(std::wstring FileName, SHADER_FLAGS flags) override;
		virtual void CreateConstantBufferToMeshRes(FMeshRes* MeshRes) override;

		// draw
		virtual void FrameBegin() override;
		virtual void DrawScene(FScene Scene) override;
		virtual void DrawActor(FActor* Actor) override;
		virtual void FrameEnd() override;

		// sync
		virtual void SyncFrame() override;

	private:
		inline void GetBackBufferIndex() { BackFrameIndex = RHISwapChain->GetCurrentBackBufferIndex(); }
		void ReadStaticMeshBinary(const std::string& BinFileName, UINT8*& PVertData, UINT8*& PIndtData, int& VertexBufferSize, int& VertexStride, int& IndexBufferSize, int& IndexNum);
		void WaitForPreviousFrame();
		void UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, FDX12Mesh* FMeshPtr);
		void UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, FDX12Mesh* FMeshPtr);
		void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps);
		void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount);
		void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap);
		void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height);
		void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version);
		UINT GetEnableShaderDebugFlags();
		D3D12_RASTERIZER_DESC CreateRasterizerStateDesc();
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
		D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const FDX12PSOInitializer& Initializer,
			ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS);
		ComPtr<ID3D12PipelineState> CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc);
		void DX12UpdateConstantBuffer(FDX12MeshRes* DX12MeshRes, ComPtr<ID3D12DescriptorHeap>& Heap);
		ComPtr<ID3D12RootSignature> CreateDX12RootSig_1CB_VS();
		void CreateGPUFence(ComPtr<ID3D12Fence>& Fence);
		void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

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
		//ComPtr<ID3D12RootSignature> RootSignature;
		ComPtr<ID3D12DescriptorHeap> RTVHeap;
		ComPtr<ID3D12DescriptorHeap> DSVHeap;
		ComPtr<ID3D12DescriptorHeap> CBVSRVHeap;
		UINT BackFrameIndex;
		HANDLE FenceEvent;
		UINT64 FenceValue;
		ComPtr<ID3D12Fence> Fence;
		UINT ResoWidth;
		UINT ResoHeight;

		// may changes attributes
		ComPtr<ID3D12Resource> DepthStencil;
		//ComPtr<ID3D12PipelineState> PipelineStateArray[10];
		//ComPtr<ID3D12Resource> ConstantBuffer;
		//ComPtr<ID3DBlob> VertexShader;
		//ComPtr<ID3DBlob> PixelShader;
	};
}
