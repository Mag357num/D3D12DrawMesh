#pragma once
#include "stdafx.h"

namespace RHI
{
	using namespace DirectX;

	class FDynamicRHI;
	class D3D12DynamicRHI;

	/** A global pointer to the dynamically bound RHI implementation. */
	extern std::shared_ptr<FDynamicRHI> GDynamicRHI;

	struct FConstantBufferBase
	{
	};

	//struct FInputLayout
	//{

	//};

	//struct FRHIShader
	//{

	//};

	//struct FDX12Shader
	//{
	//	ID3D12RootSignature* RootSignature;
	//	D3D12_SHADER_BYTECODE VS;
	//	D3D12_SHADER_BYTECODE PS;
	//};

	struct FRHIPSOInitializer
	{
		virtual void InitPsoInitializer(/*FInputLayout InputLayout, FRHIShader Shader*/) = 0;

	};

	struct FDX12PSOInitializer : public FRHIPSOInitializer
	{
	public:
		FDX12PSOInitializer();
		FDX12PSOInitializer(const D3D12_INPUT_LAYOUT_DESC& VertexDescription, /*ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS,*/
			const D3D12_RASTERIZER_DESC& rasterizerStateDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);

		void InitPsoInitializer() override
		{

		}

		struct DXGI_SAMPLE_DESC
		{
			UINT Count;
			UINT Quality;
		};

		D3D12_INPUT_LAYOUT_DESC InputLayout;
		//ID3D12RootSignature* pRootSignature;
		//D3D12_SHADER_BYTECODE VS;
		//D3D12_SHADER_BYTECODE PS;
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

	enum
	{
		BUFFRING_NUM = 2,
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

		static FDynamicRHI* DRHI;

		/* new recognize */
		static void CreateRHI();

		virtual void RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight) = 0; // factory, device, command, swapchain,
		
		//virtual void CreateShader() = 0; // root signature,
		//virtual void RHICreatePipleline() = 0; // heaps, views, pso, constantbuffer,
		//virtual void CreateMesh() = 0; // vertex buffer, index buffer
		//virtual void Draw() = 0; // populateCommandlist, fence

		virtual void GetBackBufferIndex() = 0;

		//update resource
		virtual void CreateRenderTarget() = 0;
		virtual void UpLoadConstantBuffer(const UINT& CBSize, const FConstantBufferBase& CBData, UINT8*& PCbvDataBegin) = 0; // Up Load ConstantBufferView To Heap

		// pipeline
		virtual void InitPipeLine() = 0;


		/* old recognize, which is wrong */
		virtual void EnableDebug(UINT& DxgiFactoryFlags) = 0;
		virtual void CreateFactory(bool FactoryFlags) = 0; //TODO: dont know is there a factory notion in other api, so put it here right now.
		virtual void CreateDevice(bool HasWarpDevice) = 0;

		virtual void CreateCommandQueue() = 0;
		virtual ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() = 0;
		virtual ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> CommandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState) = 0;
		virtual void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> CommandList) = 0;
		virtual void UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer, ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData) = 0;
		virtual void UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData) = 0;
		virtual void ExecuteCommand(ID3D12CommandList* PpCommandLists) = 0;

		virtual void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height) = 0;
		virtual void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps) = 0;
		virtual void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount) = 0;
		virtual void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap) = 0;
		virtual void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height) = 0;
		virtual void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version) = 0;

		virtual UINT GetEnableShaderDebugFlags() = 0;
		virtual void CreateVertexShader(LPCWSTR FileName) = 0;
		virtual void CreatePixelShader(LPCWSTR FileName) = 0;
		virtual D3D12_RASTERIZER_DESC CreateRasterizerStateDesc() = 0;
		virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc() = 0;
		virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const FDX12PSOInitializer& Initializer,
			ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS) = 0;
		virtual ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc) = 0;
		virtual void CreateDX12RootSignature() = 0;
		virtual void CreateGPUFence(ComPtr<ID3D12Fence>& Fence) = 0;

		virtual void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false) = 0;

	public:
		virtual ComPtr<ID3D12DescriptorHeap>& GetRTVHeapRef() = 0;
		virtual ComPtr<ID3D12DescriptorHeap>& GetDSVHeapRef() = 0;
		virtual ComPtr<ID3D12DescriptorHeap>& GetCBVSRVHeapRef() = 0;
		virtual ComPtr<ID3D12Device>& GetDeviceRef() = 0;
		virtual ComPtr<ID3D12CommandQueue>& GetCommandQueueRef() = 0;
		virtual ComPtr<ID3D12RootSignature>& GetRootSignatureRef() = 0;
		virtual ComPtr<ID3D12Resource>& GetVertexBufferRef() = 0;
		virtual ComPtr<ID3D12Resource>& GetIndexBufferRef() = 0;
		virtual ComPtr<ID3D12Resource>& GetConstantBufferRef() = 0;
		virtual ComPtr<ID3D12Resource>* GetRTVRef() = 0;
		virtual ComPtr<ID3D12Resource>& GetDSVRef() = 0;
		virtual D3D12_VERTEX_BUFFER_VIEW& GetVBVRef() = 0;
		virtual D3D12_INDEX_BUFFER_VIEW& GetIBVRef() = 0;
		virtual ComPtr<IDXGISwapChain3>& GetSwapChainRef() = 0;
		virtual UINT& GetBackBufferIndexRef() = 0;
		virtual ComPtr<ID3DBlob> GetVS() = 0;
		virtual ComPtr<ID3DBlob> GetPS() = 0;
		virtual ComPtr<ID3D12PipelineState>* GetPSOArray() = 0;

	protected:
		FRHIPSOInitializer* PsoInitializer;
	};

	class D3D12DynamicRHI : public FDynamicRHI
	{
	public:
		D3D12DynamicRHI();
		~D3D12DynamicRHI() = default;

		/* new recognize */
		void RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight) override
		{
			CreateDevice(UseWarpDevice);
			CreateCommandQueue();
			CreateSwapChain(BufferFrameCount, ResoWidth, ResoHeight);
			GetBackBufferIndex();

			// heaps
			CreateRenderTarget();
			CreateDescriptorHeaps(MAX_HEAP_SRV_CBV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, CBVSRVHeap); // TODO: use max amout
			CreateDescriptorHeaps(MAX_HEAP_DEPTHSTENCILS, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, DSVHeap);
			CreateDSVToHeaps(DepthStencil, DSVHeap, ResoWidth, ResoHeight);

			// command
			FCommandListDx12 CommandList;
			CommandList.Create(Device);
			GraphicsCommandLists.push_back(CommandList);

			//root signature
			CreateDX12RootSignature();

			// pso initializer
			PsoInitializer = new FDX12PSOInitializer();
		}


		//void CreateShader() override {};
		//void RHICreatePipleline() override {};
		//void CreateMesh() override {};
		//void Draw() override {};
		virtual inline void GetBackBufferIndex() { BackFrameIndex = GDynamicRHI->GetSwapChainRef()->GetCurrentBackBufferIndex(); }

		//update resource
		void CreateVertexShader(LPCWSTR FileName) override;
		void CreatePixelShader(LPCWSTR FileName) override;
		void CreateRenderTarget() override
		{
			GDynamicRHI->CreateDescriptorHeaps(BUFFRING_NUM, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, RTVHeap); //TODO: change the hard coding to double buffing.
			GDynamicRHI->CreateRTVToHeaps( RTVHeap, BUFFRING_NUM);
		}
		void UpLoadConstantBuffer(const UINT& CBSize, const FConstantBufferBase& CBData, UINT8*& PCbvDataBegin) override
		{
			DX12UpdateConstantBuffer(ConstantBuffer, CBSize, CBData, CBVSRVHeap, PCbvDataBegin);
		}

		// pipeline
		void InitPipeLine() override
		{
			FDX12PSOInitializer* Dx12Initializer = dynamic_cast<FDX12PSOInitializer*>(PsoInitializer);
			D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = GDynamicRHI->CreateGraphicsPipelineStateDesc(*Dx12Initializer,
				GDynamicRHI->GetRootSignatureRef().Get(), CD3DX12_SHADER_BYTECODE(GDynamicRHI->GetVS().Get()),
				CD3DX12_SHADER_BYTECODE(GDynamicRHI->GetPS().Get()));
			PipelineStateArray[0] = GDynamicRHI->CreateGraphicsPipelineState(PsoDesc); // TODO: hard coding
		}


		/* old recognize, which is wrong */
		void EnableDebug(UINT& DxgiFactoryFlags) override;
		void CreateFactory(bool FactoryFlags) override;
		void CreateDevice(bool HasWarpDevice) override;

		void CreateCommandQueue() override;
		ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() override;
		ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> CommandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState) override;
		void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> CommandList) override;
		void UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer, ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData) override;
		void UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData) override;
		void ExecuteCommand(ID3D12CommandList* PpCommandLists) { }; // TODO: finish this

		void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height) override;
		void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps) override;
		void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount) override;
		void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap) override;
		void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height) override;
		void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version) override;

		UINT GetEnableShaderDebugFlags() override;

		D3D12_RASTERIZER_DESC CreateRasterizerStateDesc() override;
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc() override;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const FDX12PSOInitializer& Initializer,
			ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS) override;
		ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc) override;
		void DX12UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize, const FConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin);
		void CreateDX12RootSignature() override;
		void CreateGPUFence(ComPtr<ID3D12Fence>& Fence) override;

		void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false) override;

	public:
		ComPtr<ID3D12DescriptorHeap>& GetRTVHeapRef() { return RTVHeap; }
		ComPtr<ID3D12DescriptorHeap>& GetDSVHeapRef() { return DSVHeap; }
		ComPtr<ID3D12DescriptorHeap>& GetCBVSRVHeapRef() { return CBVSRVHeap; }
		ComPtr<ID3D12Device>& GetDeviceRef() { return Device; };
		ComPtr<ID3D12CommandQueue>& GetCommandQueueRef() { return CommandQueue; }
		ComPtr<ID3D12RootSignature>& GetRootSignatureRef() { return RootSignature; };
		ComPtr<ID3D12Resource>& GetVertexBufferRef() { return VertexBuffer; };
		ComPtr<ID3D12Resource>& GetIndexBufferRef() { return IndexBuffer; };
		ComPtr<ID3D12Resource>& GetConstantBufferRef() { return ConstantBuffer; };
		ComPtr<ID3D12Resource>* GetRTVRef() { return RenderTargets; }
		ComPtr<ID3D12Resource>& GetDSVRef() { return DepthStencil; }
		D3D12_VERTEX_BUFFER_VIEW& GetVBVRef() { return VertexBufferView; }
		D3D12_INDEX_BUFFER_VIEW& GetIBVRef() { return IndexBufferView; }
		ComPtr<IDXGISwapChain3>& GetSwapChainRef() { return SwapChain; }
		UINT& GetBackBufferIndexRef() { return BackFrameIndex; }
		ComPtr<ID3DBlob> GetVS() { return VertexShader; }
		ComPtr<ID3DBlob> GetPS() { return PixelShader; }
		ComPtr<ID3D12PipelineState>* GetPSOArray() { return PipelineStateArray; }



		struct FCommandListDx12
		{
			ComPtr<ID3D12CommandAllocator> Allocators[BUFFRING_NUM]; // TODO: per commandlist with BUFFRING_NUM allocators, why?
			ComPtr<ID3D12GraphicsCommandList> CommandList;
			//ComPtr<ID3D12Fence> Fence; // TODO: thread sync?

			void Create(ComPtr<ID3D12Device> Device)
			{
				for (int i = 0; i < BUFFRING_NUM; i++)
				{
					Allocators[i] = GDynamicRHI->CreateCommandAllocator();
				}
				ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocators[0].Get(), nullptr, IID_PPV_ARGS(&CommandList)));
				CommandList->Close();
			}

			void Reset()
			{

			}
		};
		std::vector<FCommandListDx12> GraphicsCommandLists;

	private:
		ComPtr<ID3D12Device> Device;
		ComPtr<IDXGIFactory4> Factory;
		ComPtr<IDXGISwapChain3> SwapChain;
		ComPtr<ID3D12CommandQueue> CommandQueue;

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
		HANDLE FenceEventRHI;
		UINT64 GPUFenceValueRHI;
		ComPtr<ID3D12Fence> FenceGPURHI;

	};
}
