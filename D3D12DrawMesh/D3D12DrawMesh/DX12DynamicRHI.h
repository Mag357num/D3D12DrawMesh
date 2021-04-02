#pragma once
#include "stdafx.h"
#include "DynamicRHI.h"

namespace RHI
{
	using namespace DirectX;

	struct FDX12Mesh : public FMesh
	{
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
	};

	struct FCommandListDx12
	{
		ComPtr<ID3D12CommandAllocator> Allocators[BUFFRING_NUM]; // TODO: per commandlist with BUFFRING_NUM allocators, why?
		ComPtr<ID3D12GraphicsCommandList> CommandList;
		//ComPtr<ID3D12Fence> Fence; // TODO: thread sync?

		void Create(ComPtr<ID3D12Device> Device);
		void Reset(ComPtr<ID3D12PipelineState>* PipelineStateArray);
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

		/* new recognize */
		void RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight) override;

		virtual inline void GetBackBufferIndex() { BackFrameIndex = SwapChain->GetCurrentBackBufferIndex(); }

		//update resource
		void CreateVertexShader(LPCWSTR FileName) override;
		void CreatePixelShader(LPCWSTR FileName) override;
		void CreateRenderTarget() override;
		void UpLoadConstantBuffer(const UINT& CBSize, const FConstantBufferBase& CBData, UINT8*& PCbvDataBegin) override;

		// pipeline
		void InitPipeLine() override;

		std::vector<FCommandListDx12> GraphicsCommandLists;

		// mesh
		FMesh* CreateMesh(const std::string& BinFileName) override;
		void UpLoadMesh(FMesh* Mesh) override;
		void ReadStaticMeshBinary(const std::string& BinFileName, UINT8*& PVertData, UINT8*& PIndtData, int& VertexBufferSize, int& VertexStride, int& IndexBufferSize, int& IndexNum);

		// draw
		void PopulateCommandList(FDX12Mesh* MeshPtr);
		void WaitForPreviousFrame();
		void DrawMesh(FMesh* MeshPtr) override;

		// sync
		void SyncFrame() override;

		/* old recognize, which is wrong */
		void EnableDebug(UINT& DxgiFactoryFlags);
		void CreateFactory(bool FactoryFlags);
		void CreateDevice(bool HasWarpDevice);

		void CreateCommandQueue();
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> CommandAllocator);
		void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> CommandList);
		void UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer,
			ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData);
		void UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer,
			ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData);
		void ExecuteCommand(ID3D12CommandList* PpCommandLists) { }; // TODO: finish this

		void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height);
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
		ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc);
		void DX12UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize, const FConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin);
		void CreateDX12RootSignature();
		void CreateGPUFence(ComPtr<ID3D12Fence>& Fence);

		void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);

	private:
		ComPtr<ID3D12Device> Device;
		ComPtr<IDXGIFactory4> Factory;
		ComPtr<IDXGISwapChain3> SwapChain;
		ComPtr<ID3D12CommandQueue> CommandQueue;
		D3D12_VIEWPORT Viewport;
		D3D12_RECT ScissorRect;

		// Pipeline objects.
		ComPtr<ID3D12Resource> RenderTargets[3]; // TODO: hard coding to 3
		ComPtr<ID3D12RootSignature> RootSignature;
		ComPtr<ID3D12DescriptorHeap> RTVHeap;
		ComPtr<ID3D12DescriptorHeap> DSVHeap;
		ComPtr<ID3D12DescriptorHeap> CBVSRVHeap;
		ComPtr<ID3D12Resource> DepthStencil;
		ComPtr<ID3D12PipelineState> PipelineStateArray[10];

		// App resources.
		ComPtr<ID3D12Resource> VertexBuffer;
		ComPtr<ID3D12Resource> IndexBuffer;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;
		ComPtr<ID3D12Resource> ConstantBuffer;
		ComPtr<ID3D12Resource> VertexBufferUploadHeap;
		ComPtr<ID3D12Resource> IndexBufferUploadHeap;
		ComPtr<ID3DBlob> VertexShader;
		ComPtr<ID3DBlob> PixelShader;

		// Synchronization objects.
		UINT BackFrameIndex;
		HANDLE FenceEvent;
		UINT64 FenceValue;
		ComPtr<ID3D12Fence> Fence;

		UINT ResoWidth;
		UINT ResoHeight;
	};
}
