#include "DX12DynamicRHI.h"
#include "DXSampleHelper.h"
#include "Renderer.h"
#include "dxgidebug.h"

namespace RHI
{
	FDX12PSOInitializer::FDX12PSOInitializer()
	{
		static D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		CD3DX12_RASTERIZER_DESC rasterizerStateDesc(D3D12_DEFAULT);
		rasterizerStateDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerStateDesc.FrontCounterClockwise = TRUE;

		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = TRUE;

		InputLayout = { InputElementDescs, _countof(InputElementDescs) };
		RasterizerState = rasterizerStateDesc;
		BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		DepthStencilState = depthStencilDesc;
		SampleMask = UINT_MAX;
		PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		NumRenderTargets = 1;
		RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		DSVFormat = DXGI_FORMAT_D32_FLOAT;
		SampleDesc.Count = 1;
	}

	FDX12PSOInitializer::FDX12PSOInitializer(const D3D12_INPUT_LAYOUT_DESC& VertexDescription,
		const D3D12_RASTERIZER_DESC& rasterizerStateDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc)
	{
		InputLayout = VertexDescription;
		RasterizerState = rasterizerStateDesc;
		BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		DepthStencilState = depthStencilDesc;
		SampleMask = UINT_MAX;
		PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		NumRenderTargets = 1;
		RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		DSVFormat = DXGI_FORMAT_D32_FLOAT;
		SampleDesc.Count = 1;
	}

	void FDX12DynamicRHI::RHIInit(bool UseWarpDevice, UINT BufferFrameCount, UINT ResoWidth, UINT ResoHeight)
	{
		this->ResoWidth = ResoWidth;
		this->ResoHeight = ResoHeight;

		Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(ResoWidth), static_cast<float>(ResoHeight));
		ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(ResoWidth), static_cast<LONG>(ResoHeight));

		//create device
		if (UseWarpDevice)
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed(Factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

			ThrowIfFailed(D3D12CreateDevice(
				warpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&Device)
			));
		}
		else
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter(Factory.Get(), &hardwareAdapter);

			ThrowIfFailed(D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&Device)
			));
		}

		// Try to create debug factory
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue))))
		{
			CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&DxgiFactory));

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
		else
		{
			// Failed to create debug factory, create a normal one
			CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));
		}


		// command queue
		D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)));

		//swapchain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = BufferFrameCount;
		swapChainDesc.Width = ResoWidth;
		swapChainDesc.Height = ResoHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(Factory->CreateSwapChainForHwnd(
			CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			Renderer::GetHwnd(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		ThrowIfFailed(Factory->MakeWindowAssociation(Renderer::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
		ThrowIfFailed(swapChain.As(&SwapChain)); // convert different version of swapchain type
		GetBackBufferIndex();

		// heaps
		CreateDescriptorHeaps(BUFFRING_NUM, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, RTVHeap); //TODO: change the hard coding to double buffing.
		CreateRTVToHeaps(RTVHeap, BUFFRING_NUM);
		CreateDescriptorHeaps(MAX_HEAP_SRV_CBV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, CBVSRVHeap); // TODO: use max amout
		CreateDescriptorHeaps(MAX_HEAP_DEPTHSTENCILS, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, DSVHeap);
		CreateDSVToHeaps(DepthStencil, DSVHeap, ResoWidth, ResoHeight);

		// command
		FCommandListDx12 DrawCommandList;
		DrawCommandList.Create(Device);
		GraphicsCommandLists.push_back(DrawCommandList);

		GraphicsCommandLists[0].CommandList->RSSetViewports(1, &Viewport);
		GraphicsCommandLists[0].CommandList->RSSetScissorRects(1, &ScissorRect);

		//root signature
		CreateDX12RootSignature();

		// pso initializer
		PsoInitializer = new FDX12PSOInitializer();
	}

	void FCommandListDx12::Reset(ComPtr<ID3D12PipelineState>* PipelineStateArray)
	{
		for (int i = 0; i < BUFFRING_NUM; i++)
		{
			ThrowIfFailed(Allocators[i]->Reset());
		}

		ThrowIfFailed(CommandList->Reset(Allocators[0].Get(), PipelineStateArray[0].Get()));
	}

	void FCommandListDx12::Create(ComPtr<ID3D12Device> Device)
	{
		for (int i = 0; i < BUFFRING_NUM; i++)
		{
			ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocators[i])));
		}
		ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocators[0].Get(), nullptr, IID_PPV_ARGS(&CommandList)));
	}

	void FDX12DynamicRHI::UpLoadConstantBuffer(const UINT& CBSize, const FConstantBufferBase& CBData, UINT8*& PCbvDataBegin)
	{
		DX12UpdateConstantBuffer(ConstantBuffer, CBSize, CBData, CBVSRVHeap, PCbvDataBegin);
	}

	void FDX12DynamicRHI::InitPipeLine()
	{
		FDX12PSOInitializer* Dx12Initializer = dynamic_cast<FDX12PSOInitializer*>(PsoInitializer);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = CreateGraphicsPipelineStateDesc(*Dx12Initializer,
			RootSignature.Get(), CD3DX12_SHADER_BYTECODE(VertexShader.Get()),
			CD3DX12_SHADER_BYTECODE(PixelShader.Get()));
		PipelineStateArray[0] = CreateGraphicsPipelineState(PsoDesc); // TODO: hard coding
	}

	FDX12DynamicRHI::FDX12DynamicRHI()
	{
		UINT dxgiFactoryFlags = 0;
		#if defined(_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				// Enable additional debug layers.
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
		#endif
		ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&Factory)));
	}

	void FDX12DynamicRHI::UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, FDX12Mesh* FMeshPtr)
	{
		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(FMeshPtr->VertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&FMeshPtr->VertexBuffer)));

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(FMeshPtr->VertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&FMeshPtr->VertexBufferUploadHeap)));

		NAME_D3D12_OBJECT(FMeshPtr->VertexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = FMeshPtr->PVertData;
		vertexData.RowPitch = FMeshPtr->VertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(CommandList.Get(), FMeshPtr->VertexBuffer.Get(), FMeshPtr->VertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(FMeshPtr->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		FMeshPtr->VertexBufferView.BufferLocation = FMeshPtr->VertexBuffer->GetGPUVirtualAddress();
		FMeshPtr->VertexBufferView.StrideInBytes = FMeshPtr->VertexStride;
		FMeshPtr->VertexBufferView.SizeInBytes = FMeshPtr->VertexBufferSize;
	}

	void FDX12DynamicRHI::UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, FDX12Mesh* FMeshPtr)
	{
		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(FMeshPtr->IndexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&FMeshPtr->IndexBuffer)));

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(FMeshPtr->IndexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&FMeshPtr->IndexBufferUploadHeap)));

		NAME_D3D12_OBJECT(FMeshPtr->IndexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = FMeshPtr->PIndtData;
		indexData.RowPitch = FMeshPtr->IndexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;

		UpdateSubresources<1>(CommandList.Get(), FMeshPtr->IndexBuffer.Get(), FMeshPtr->IndexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(FMeshPtr->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		// Describe the index buffer view.
		FMeshPtr->IndexBufferView.BufferLocation = FMeshPtr->IndexBuffer->GetGPUVirtualAddress();
		FMeshPtr->IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		FMeshPtr->IndexBufferView.SizeInBytes = FMeshPtr->IndexBufferSize;
	}

	void FDX12DynamicRHI::CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps)
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = NumDescriptors;
		HeapDesc.Type = Type;
		HeapDesc.Flags = Flags;
		ThrowIfFailed(Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeaps)));
	}

	void FDX12DynamicRHI::CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE HeapsHandle(Heap->GetCPUDescriptorHandleForHeapStart());

		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n])));
			Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, HeapsHandle);
			HeapsHandle.Offset(1, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		}
	}

	void FDX12DynamicRHI::CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap)
	{
		Device->CreateConstantBufferView(&CbvDesc, Heap->GetCPUDescriptorHandleForHeapStart());
	}

	void FDX12DynamicRHI::CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&DepthStencilBuffer)
		));

		NAME_D3D12_OBJECT(DepthStencilBuffer);

		Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &depthStencilDesc, Heap->GetCPUDescriptorHandleForHeapStart());
	}

	void FDX12DynamicRHI::ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version)
	{
		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
	}

	UINT FDX12DynamicRHI::GetEnableShaderDebugFlags()
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		return compileFlags;
	}

	void FDX12DynamicRHI::CreateVertexShader(LPCWSTR FileName)
	{
		ThrowIfFailed(D3DCompileFromFile(FileName, nullptr, nullptr, "VSMain", "vs_5_0", GetEnableShaderDebugFlags(), 0, &VertexShader, nullptr));
	}

	void FDX12DynamicRHI::CreatePixelShader(LPCWSTR FileName)
	{
		ThrowIfFailed(D3DCompileFromFile(FileName, nullptr, nullptr, "PSMain", "ps_5_0", GetEnableShaderDebugFlags(), 0, &PixelShader, nullptr));
	}

	D3D12_RASTERIZER_DESC FDX12DynamicRHI::CreateRasterizerStateDesc()
	{
		CD3DX12_RASTERIZER_DESC rasterizerStateDesc(D3D12_DEFAULT);
		rasterizerStateDesc.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerStateDesc.FrontCounterClockwise = TRUE;
		return static_cast<D3D12_RASTERIZER_DESC>(rasterizerStateDesc);
	}

	D3D12_DEPTH_STENCIL_DESC FDX12DynamicRHI::CreateDepthStencilDesc()
	{
		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = TRUE;
		return static_cast<D3D12_DEPTH_STENCIL_DESC>(depthStencilDesc);
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC FDX12DynamicRHI::CreateGraphicsPipelineStateDesc(const FDX12PSOInitializer& Initializer,
		ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS, const D3D12_SHADER_BYTECODE& PS)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = Initializer.InputLayout;
		psoDesc.pRootSignature = RootSignature;
		psoDesc.VS = VS;
		psoDesc.PS = PS;
		psoDesc.RasterizerState = Initializer.RasterizerState;
		psoDesc.BlendState = Initializer.BlendState;
		psoDesc.DepthStencilState = Initializer.DepthStencilState;
		psoDesc.SampleMask = Initializer.SampleMask;
		psoDesc.PrimitiveTopologyType = Initializer.PrimitiveTopologyType;
		psoDesc.NumRenderTargets = Initializer.NumRenderTargets;
		psoDesc.RTVFormats[0] = Initializer.RTVFormats[0];
		//psoDesc.RTVFormats = Initializer.RTVFormats; // TODO: why error?
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = Initializer.SampleDesc.Count;
		return psoDesc;
	}

	ComPtr<ID3D12PipelineState> FDX12DynamicRHI::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc)
	{
		ComPtr<ID3D12PipelineState> PipelineState;
		ThrowIfFailed(Device->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PipelineState)));
		return PipelineState;
	}

	void FDX12DynamicRHI::DX12UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize,
		const FConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin)
	{
		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(ConstantBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&ConstantBuffer)));

		NAME_D3D12_OBJECT(ConstantBuffer);

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc = {};
		CbvDesc.BufferLocation = ConstantBuffer->GetGPUVirtualAddress();
		CbvDesc.SizeInBytes = ConstantBufferSize;
		CreateCBVToHeaps(CbvDesc, Heap);

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		// Map: resource give cpu the right to dynamic manipulate(memcpy) it, and forbid gpu to manipulate it, until Unmap occur.
		// resource->Map(subresource, cpuReadRange, cpuVirtualAdress )
		ThrowIfFailed(ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&PCbvDataBegin)));
		memcpy(PCbvDataBegin, &ConstantBufferData, ConstantBufferSize);
	}

	_Use_decl_annotations_
		void FDX12DynamicRHI::GetHardwareAdapter(
			IDXGIFactory1* pFactory,
			IDXGIAdapter1** ppAdapter,
			bool requestHighPerformanceAdapter)
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			for (
				UINT adapterIndex = 0;
				DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
					IID_PPV_ARGS(&adapter));
				++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}
		else
		{
			for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}

	void FDX12DynamicRHI::CreateDX12RootSignature()
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};
		ChooseSupportedFeatureVersion(FeatureData, D3D_ROOT_SIGNATURE_VERSION_1_1);

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> Signature;
		ComPtr<ID3DBlob> Error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, FeatureData.HighestVersion, &Signature, &Error));
		ThrowIfFailed(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
	}

	void FDX12DynamicRHI::CreateGPUFence(ComPtr<ID3D12Fence>& Fence)
	{
		ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	}

	void FDX12DynamicRHI::ReadStaticMeshBinary(const std::string& BinFileName, UINT8*& PVertData, UINT8*& PIndtData, int& VertexBufferSize, int& VertexStride, int& IndexBufferSize, int& IndexNum)
	{
		std::ifstream Fin(BinFileName, std::ios::binary);

		if (!Fin.is_open())
		{
			throw std::exception("open file faild.");
		}

		Fin.read((char*)&VertexStride, sizeof(int));

		Fin.read((char*)&VertexBufferSize, sizeof(int));
		VertexBufferSize *= static_cast<size_t>(VertexStride);

		if (VertexBufferSize > 0)
		{
			PVertData = reinterpret_cast<UINT8*>(malloc(VertexBufferSize));
			Fin.read((char*)PVertData, VertexBufferSize);
		}
		else
		{
			throw std::exception();
		}

		Fin.read((char*)&IndexBufferSize, sizeof(int));
		IndexNum = IndexBufferSize;
		IndexBufferSize *= sizeof(int);

		if (IndexBufferSize > 0)
		{
			PIndtData = reinterpret_cast<UINT8*>(malloc(IndexBufferSize));
			Fin.read((char*)PIndtData, IndexBufferSize);
		}

		Fin.close();
	}

	void FDX12DynamicRHI::UpLoadMesh(FMesh* Mesh)
	{
		FDX12Mesh* DX12Mesh = dynamic_cast<FDX12Mesh*>(Mesh);
		UpdateVertexBuffer(GraphicsCommandLists[0].CommandList, DX12Mesh);
		UpdateIndexBuffer(GraphicsCommandLists[0].CommandList, DX12Mesh);
		
		GraphicsCommandLists[0].CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { GraphicsCommandLists[0].CommandList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	FMesh* FDX12DynamicRHI::CreateMesh(const std::string& BinFileName)
	{
		FMesh* MeshPtr = new FDX12Mesh();
		ReadStaticMeshBinary(BinFileName, MeshPtr->PVertData, MeshPtr->PIndtData, MeshPtr->VertexBufferSize, MeshPtr->VertexStride, MeshPtr->IndexBufferSize, MeshPtr->IndexNum);
		return MeshPtr;
	}

	void FDX12DynamicRHI::FrameBegin()
	{
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart(), BackFrameIndex, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		CD3DX12_CPU_DESCRIPTOR_HANDLE DsvHandle(DSVHeap->GetCPUDescriptorHandleForHeapStart());
		GraphicsCommandLists[0].CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[BackFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		GraphicsCommandLists[0].CommandList->OMSetRenderTargets(1, &RtvHandle, FALSE, &DsvHandle);
		GraphicsCommandLists[0].CommandList->ClearRenderTargetView(RtvHandle, clearColor, 0, nullptr);
		GraphicsCommandLists[0].CommandList->ClearDepthStencilView(DsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void FDX12DynamicRHI::DrawMesh(FMesh* MeshPtr)
	{
		FDX12Mesh* DX12Mesh = dynamic_cast<FDX12Mesh*>(MeshPtr);

		GraphicsCommandLists[0].CommandList->SetGraphicsRootSignature(RootSignature.Get()); // TODO: need per frame?
		ID3D12DescriptorHeap* ppHeaps[] = { CBVSRVHeap.Get() };
		GraphicsCommandLists[0].CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		CD3DX12_GPU_DESCRIPTOR_HANDLE CbvHandle(CBVSRVHeap->GetGPUDescriptorHandleForHeapStart());
		GraphicsCommandLists[0].CommandList->SetGraphicsRootDescriptorTable(0, CbvHandle);
		GraphicsCommandLists[0].CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GraphicsCommandLists[0].CommandList->IASetIndexBuffer(&DX12Mesh->IndexBufferView);
		GraphicsCommandLists[0].CommandList->IASetVertexBuffers(0, 1, &DX12Mesh->VertexBufferView);
		GraphicsCommandLists[0].CommandList->DrawIndexedInstanced(DX12Mesh->IndexNum, 1, 0, 0, 0);
	}

	void FDX12DynamicRHI::FrameEnd()
	{
		GraphicsCommandLists[0].CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[BackFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// Execute the command list.
		GraphicsCommandLists[0].CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { GraphicsCommandLists[0].CommandList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Present the frame.
		ThrowIfFailed(SwapChain->Present(1, 0));
		WaitForPreviousFrame();
		GraphicsCommandLists[0].Reset(PipelineStateArray);
	}

	void FDX12DynamicRHI::WaitForPreviousFrame()
	{
		const UINT64 fence = FenceValue; //m_fenceValue: CPU fence value
		ThrowIfFailed(CommandQueue->Signal(Fence.Get(), fence)); // set a fence in GPU
		FenceValue++;

		if (Fence->GetCompletedValue() < fence) // if GPU run after CPU, make CPU wait for GPU
		{
			ThrowIfFailed(Fence->SetEventOnCompletion(fence, FenceEvent)); // define m_fenceEvent as the event that fire when m_fence hit the fence param
			WaitForSingleObject(FenceEvent, INFINITE); // CPU wait
		}

		BackFrameIndex = SwapChain->GetCurrentBackBufferIndex();
	}

	void FDX12DynamicRHI::SyncFrame()
	{
		CreateGPUFence(Fence);
		FenceValue = 1;

		// Create an event handle to use for frame synchronization.
		FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (FenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		WaitForPreviousFrame();
		GraphicsCommandLists[0].Reset(PipelineStateArray);
	}

}
