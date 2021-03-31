#pragma once
#include "stdafx.h"
#include "Win32Application.h" // TODO: put this into stdafx.h

namespace RHI
{
	using namespace DirectX;

	class DynamicRHI;

	/** A global pointer to the dynamically bound RHI implementation. */
	extern DynamicRHI* GDynamicRHI;

	struct ConstantBufferBase
	{
	};

	// TODO: make pipeline state initializer a API-independent pso initializer (should change the member to API-independent)
	struct GraphicsPipelineStateInitializer
	{
	public:
		GraphicsPipelineStateInitializer() = default;
		GraphicsPipelineStateInitializer(const D3D12_INPUT_LAYOUT_DESC& VertexDescription, ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS, const D3D12_RASTERIZER_DESC& rasterizerStateDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);

		struct DXGI_SAMPLE_DESC
		{
			UINT Count;
			UINT Quality;
		};

		D3D12_INPUT_LAYOUT_DESC InputLayout;
		ID3D12RootSignature* pRootSignature;
		D3D12_SHADER_BYTECODE VS;
		D3D12_SHADER_BYTECODE PS;
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

	class DynamicRHI
	{
	public:
		DynamicRHI() = default;
		~DynamicRHI() = default;

		virtual void EnableDebug(UINT& DxgiFactoryFlags) = 0;
		virtual void CreateFactory(bool FactoryFlags) = 0; //TODO: dont know is there a factory notion in other api, so put it here right now.
		virtual void CreateDevice(bool HasWarpDevice) = 0;

		virtual void CreateCommandQueue() = 0;
		virtual ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() = 0;
		virtual ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState) = 0;
		virtual void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) = 0;
		virtual void UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer, ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData) = 0;
		virtual void UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData) = 0;
		virtual void ExecuteCommand(ID3D12CommandList* ppCommandLists) = 0;

		virtual void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height, DXGI_FORMAT Format) = 0;
		virtual void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps) = 0;
		virtual void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount) = 0;
		virtual void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap) = 0;
		virtual void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height) = 0;
		virtual void CreateCommandlist(ComPtr<ID3D12CommandAllocator>& CommandAllocator) = 0;
		virtual void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version) = 0;

		virtual UINT GetEnableShaderDebugFlags() = 0;
		virtual ComPtr<ID3DBlob> CreateVertexShader(LPCWSTR FileName) = 0;
		virtual ComPtr<ID3DBlob> CreatePixelShader(LPCWSTR FileName) = 0;
		virtual D3D12_RASTERIZER_DESC CreateRasterizerStateDesc() = 0;
		virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc() = 0;
		virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const GraphicsPipelineStateInitializer& Initializer) = 0;
		virtual void CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ComPtr<ID3D12PipelineState>& pipelineState) = 0;
		virtual ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) = 0;
		virtual void UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize, const ConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin) = 0;
		virtual void CreateRootSignature(D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData) = 0;
		virtual void CreateGPUFence(ComPtr<ID3D12Fence>& Fence) = 0;

		virtual void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false) = 0;

	public:
		virtual ComPtr<ID3D12DescriptorHeap>& GetRTVHeap() = 0;
		virtual ComPtr<ID3D12DescriptorHeap>& GetDSVHeap() = 0;
		virtual ComPtr<ID3D12DescriptorHeap>& GetCBVHeap() = 0;
		virtual ComPtr<ID3D12Device>& GetDevice() = 0;
		virtual ComPtr<ID3D12CommandQueue>& GetCommandQueue() = 0;
		virtual ComPtr<ID3D12RootSignature>& GetRootSignature() = 0;
		virtual ComPtr<ID3D12Resource>& GetVertexBuffer() = 0;
		virtual ComPtr<ID3D12Resource>& GetIndexBuffer() = 0;
		virtual ComPtr<ID3D12Resource>& GetConstantBuffer() = 0;
		virtual ComPtr<ID3D12Resource>* GetRTV() = 0;
		virtual ComPtr<ID3D12Resource>& GetDSV() = 0;
		virtual D3D12_VERTEX_BUFFER_VIEW& GetVBV() = 0;
		virtual D3D12_INDEX_BUFFER_VIEW& GetIBV() = 0;
		virtual ComPtr<IDXGISwapChain3>& GetSwapChain() = 0;
	};

	class D3D12DynamicRHI : public DynamicRHI
	{
	public:
		D3D12DynamicRHI();
		~D3D12DynamicRHI() = default;

		void EnableDebug(UINT& DxgiFactoryFlags) override;
		void CreateFactory(bool FactoryFlags) override;
		void CreateDevice(bool HasWarpDevice) override;

		void CreateCommandQueue() override;
		void CreateCommandlist(ComPtr<ID3D12CommandAllocator>& CommandAllocator) override;
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() override;
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState) override;
		void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) override;
		void UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer, ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData) override;
		void UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData) override;
		void ExecuteCommand(ID3D12CommandList* ppCommandLists) { }; // TODO: finish this

		void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height, DXGI_FORMAT Format) override;
		void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps) override;
		void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount) override;
		void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap) override;
		void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height) override;
		void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version) override;

		UINT GetEnableShaderDebugFlags() override;
		ComPtr<ID3DBlob> CreateVertexShader(LPCWSTR FileName) override;
		ComPtr<ID3DBlob> CreatePixelShader(LPCWSTR FileName) override;
		D3D12_RASTERIZER_DESC CreateRasterizerStateDesc() override;
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc() override;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const GraphicsPipelineStateInitializer& Initializer) override;
		void CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ComPtr<ID3D12PipelineState>& pipelineState) override;
		ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) override;
		void UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize, const ConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin) override;
		void CreateRootSignature(D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData) override;
		void CreateGPUFence(ComPtr<ID3D12Fence>& Fence) override;


		void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false) override;

	public:
		ComPtr<ID3D12DescriptorHeap>& GetRTVHeap() { return m_rtvHeap; }
		ComPtr<ID3D12DescriptorHeap>& GetDSVHeap() { return m_dsvHeap; }
		ComPtr<ID3D12DescriptorHeap>& GetCBVHeap() { return m_cbvHeap; }
		ComPtr<ID3D12Device>& GetDevice() { return Device; };
		ComPtr<ID3D12CommandQueue>& GetCommandQueue() { return CommandQueue; }
		ComPtr<ID3D12RootSignature>& GetRootSignature() { return m_rootSignature; };
		ComPtr<ID3D12Resource>& GetVertexBuffer() { return m_vertexBuffer; };
		ComPtr<ID3D12Resource>& GetIndexBuffer() { return m_indexBuffer; };
		ComPtr<ID3D12Resource>& GetConstantBuffer() { return m_constantBuffer; };
		ComPtr<ID3D12Resource>* GetRTV() { return m_renderTargets; }
		ComPtr<ID3D12Resource>& GetDSV() { return m_depthStencil; }
		D3D12_VERTEX_BUFFER_VIEW& GetVBV() { return m_vertexBufferView; }
		D3D12_INDEX_BUFFER_VIEW& GetIBV() { return m_indexBufferView; }
		ComPtr<IDXGISwapChain3>& GetSwapChain() { return SwapChain; }


	private:
		ComPtr<ID3D12Device> Device;
		ComPtr<IDXGIFactory4> Factory;
		ComPtr<IDXGISwapChain3> SwapChain;
		ComPtr<ID3D12CommandQueue> CommandQueue;



		// Pipeline objects.
		ComPtr<ID3D12Resource> m_renderTargets[3]; // TODO: hard coding to 3
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
		ComPtr<ID3D12Resource> m_depthStencil;
		UINT m_cbSrvDescriptorSize;

		// App resources.
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> VertexBufferUploadHeap;
		ComPtr<ID3D12Resource> IndexBufferUploadHeap;

	};
}

