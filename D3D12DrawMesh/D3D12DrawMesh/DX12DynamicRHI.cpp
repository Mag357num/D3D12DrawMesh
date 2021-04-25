#include "DX12DynamicRHI.h"
#include "DXSampleHelper.h"
#include "dxgidebug.h"
#include "Win32Application.h"
#include "FrameResourceManager.h"

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

	void FDX12DynamicRHI::RHIInit(const bool& UseWarpDevice, const uint32& BufferFrameCount, const uint32& ResoWidth, const uint32& ResoHeight)
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
		ThrowIfFailed(Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&RHICommandQueue)));

		//swapchain
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = BufferFrameCount;
		swapChainDesc.Width = ResoWidth;
		swapChainDesc.Height = ResoHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> SwapChain;
		ThrowIfFailed(Factory->CreateSwapChainForHwnd(
			RHICommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			Win32Application::GetHwnd(),
			&swapChainDesc,
			nullptr,
			nullptr,
			&SwapChain
		));

		ThrowIfFailed(Factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
		ThrowIfFailed(SwapChain.As(&RHISwapChain)); // convert different version of swapchain type
		GetBackBufferIndex();

		// commit shadow map resource
		CommitShadowMap();

		// heaps
		CreateDescriptorHeaps(MAX_HEAP_RENDERTARGETS, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, RTVHeap); //TODO: change the hard coding to double buffing.
		CreateRTVToHeaps(2);

		CreateDescriptorHeaps(MAX_HEAP_SRV_CBV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, CBVSRVHeap); // TODO: use max amout
		LastCPUHandleForCBVSRV = CBVSRVHeap->GetCPUDescriptorHandleForHeapStart();
		LastGPUHandleForCBVSRV = CBVSRVHeap->GetGPUDescriptorHandleForHeapStart();

		CreateNullMapToCBVSRVHeaps();
		CreateShadowMapToCBVSRVHeaps();

		CreateDescriptorHeaps(MAX_HEAP_DEPTHSTENCILS, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, DSVHeap);
		LastCPUHandleForDSV = DSVHeap->GetCPUDescriptorHandleForHeapStart();
		CreateDSVToHeaps();
		CreateShadowMapToDSVHeaps();

		CreateDescriptorHeaps(MAX_HEAP_SAMPLERS, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, SamplerHeap);
		CreateSamplerToHeaps(FSamplerType::CLAMP);

		// command
		FCommand DrawCommandList(RHICommandQueue); // TODO: no need to use the FCommandListDx12, one commandlist is enough for use
		DrawCommandList.Create(Device);
		DrawCommandList.Close();
		CommandLists.push_back(DrawCommandList);

		CreateFenceAndEvent();
	}

	void FCommand::Reset()
	{
		ThrowIfFailed(Allocator->Reset());
		ThrowIfFailed(CommandList->Reset(Allocator.Get(), NULL));
	}

	void FCommand::Create(ComPtr<ID3D12Device> Device)
	{
		ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)));
		ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator.Get(), nullptr, IID_PPV_ARGS(&CommandList)));
	}

	void FCommand::Close()
	{
		ThrowIfFailed(CommandList->Close());
	}

	void FCommand::Execute()
	{
		ID3D12CommandList* ppCommandLists[] = { CommandList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	shared_ptr<RHI::FCB> FDX12DynamicRHI::CreateConstantBufferToMeshRes(const uint32& Size)
{
		shared_ptr<FDX12CB> DX12CB = make_shared<FDX12CB>();
		DX12CreateConstantBuffer(DX12CB.get(), Size);
		return DX12CB;
	}

	void FDX12DynamicRHI::UpdateConstantBufferInMeshRes(FMeshRes* MeshRes, FCBData* BaseData, FCBData* ShadowData)
	{
		FDX12CB* DX12BaseCB = dynamic_cast<FDX12CB*>(MeshRes->BaseCB.get());
		FDX12CB* DX12ShadowCB = dynamic_cast<FDX12CB*>(MeshRes->ShadowCB.get());

		memcpy(DX12BaseCB->UploadBufferVirtualAddress, BaseData->DataBuffer, BaseData->BufferSize);
		memcpy(DX12ShadowCB->UploadBufferVirtualAddress, ShadowData->DataBuffer, ShadowData->BufferSize);
	}

	void FDX12DynamicRHI::InitPipeLineToMeshRes(FMeshRes* MeshRes, FPSOInitializer* PsoInitializer, const SHADER_FLAGS& rootFlags)
	{
		FDX12MeshRes* DX12MeshRes = dynamic_cast<FDX12MeshRes*>(MeshRes);
		FShader* VS = DX12MeshRes->VS.get();
		FShader* PS = DX12MeshRes->PS.get();
		FDX12Shader* DX12VS = dynamic_cast<FDX12Shader*>(VS);
		FDX12Shader* DX12PS = dynamic_cast<FDX12Shader*>(PS);

		if (rootFlags == SHADER_FLAGS::CB0_SR1_Sa2)
		{
			DX12MeshRes->RootSignature = CreateDX12RootSig_CB0_SR1_Sa2();
		}
		else
		{
			throw std::exception(" didnt assign rootsignature! ");
		}

		// create bass pass pso
		FDX12PSOInitializer* Dx12Initializer = dynamic_cast<FDX12PSOInitializer*>(PsoInitializer);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = CreateGraphicsPipelineStateDesc(*Dx12Initializer,
			DX12MeshRes->RootSignature.Get(), CD3DX12_SHADER_BYTECODE(DX12VS->Shader.Get()),
			CD3DX12_SHADER_BYTECODE(DX12PS->Shader.Get()));
		DX12MeshRes->BasePSO = CreatePSO(PsoDesc);

		// create shadow pass pso
		PsoDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
		PsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		PsoDesc.NumRenderTargets = 0;

		DX12MeshRes->ShadowPSO = CreatePSO(PsoDesc);
	}

	FDX12DynamicRHI::FDX12DynamicRHI()
	{
		uint32 dxgiFactoryFlags = 0;
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

	void FDX12DynamicRHI::CreateDescriptorHeaps(const uint32& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps)
	{
		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = NumDescriptors;
		HeapDesc.Type = Type;
		HeapDesc.Flags = Flags;
		ThrowIfFailed(Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeaps)));
	}

	void FDX12DynamicRHI::CreateRTVToHeaps(const uint32& FrameCount)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE HeapsHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

		for (uint32 n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(RHISwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n])));
			Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, HeapsHandle);
			HeapsHandle.Offset(1, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		}
	}

	void FDX12DynamicRHI::CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& CbvDesc, FDX12CB* FDX12CB)
	{
		Device->CreateConstantBufferView(&CbvDesc, LastCPUHandleForCBVSRV);
		FDX12CB->GPUHandleInHeap = LastGPUHandleForCBVSRV;

		LastCPUHandleForCBVSRV.Offset(Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)); // TODO: have the risk of race
		LastGPUHandleForCBVSRV.Offset(Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	void FDX12DynamicRHI::CreateDSVToHeaps()
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
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, ResoWidth, ResoHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&DepthStencilBuffer)
		));

		NAME_D3D12_OBJECT(DepthStencilBuffer);

		Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &depthStencilDesc, LastCPUHandleForDSV);
		DSVHandle = LastCPUHandleForDSV;

		LastCPUHandleForDSV.Offset(Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)); // TODO: have the risk of race
	}

	void FDX12DynamicRHI::CreateSamplerToHeaps(FSamplerType Type)
	{
		D3D12_SAMPLER_DESC SamplerDesc = {};
		if (Type == FSamplerType::CLAMP)
		{
			SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		}
		else if(Type == FSamplerType::WARP)
		{
			SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
		else
		{
			throw std::exception("do not have this type of sampler");
		}
		SamplerDesc.MipLODBias = 0.0f;
		SamplerDesc.MaxAnisotropy = 1;
		SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		SamplerDesc.BorderColor[0] = SamplerDesc.BorderColor[1] = SamplerDesc.BorderColor[2] = SamplerDesc.BorderColor[3] = 0;
		SamplerDesc.MinLOD = 0;
		SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		Device->CreateSampler(&SamplerDesc, SamplerHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void FDX12DynamicRHI::CreateShadowMapToDSVHeaps()
{
		// create depth stencil view to heap
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		Device->CreateDepthStencilView(DX12ShadowMap.Get(), &depthStencilViewDesc, LastCPUHandleForDSV);
		ShadowDepthViewHandle = LastCPUHandleForDSV;
		LastCPUHandleForDSV.Offset(Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)); // TODO: have the risk of race
	}

	void FDX12DynamicRHI::CreateNullMapToCBVSRVHeaps()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
		nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		nullSrvDesc.Texture2D.MipLevels = 1;
		nullSrvDesc.Texture2D.MostDetailedMip = 0;
		nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		CreateSRVToHeaps(nullptr, nullSrvDesc, NullGPUHandle);
	}

	void FDX12DynamicRHI::CreateShadowMapToCBVSRVHeaps()
{
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		CreateSRVToHeaps(DX12ShadowMap.Get(), shadowSrvDesc, ShadowMapGPUHandleForCBVSRV);
	}

	void FDX12DynamicRHI::CreateSRVToHeaps(ID3D12Resource* ShaderResource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SrvDesc, CD3DX12_GPU_DESCRIPTOR_HANDLE& Handle)
	{
		Device->CreateShaderResourceView(ShaderResource, &SrvDesc, LastCPUHandleForCBVSRV);
		Handle = LastGPUHandleForCBVSRV;

		LastCPUHandleForCBVSRV.Offset(Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)); // TODO: have the risk of race
		LastGPUHandleForCBVSRV.Offset(Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
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

	uint32 FDX12DynamicRHI::GetEnableShaderDebugFlags()
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		uint32 compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		uint32 compileFlags = 0;
#endif
		return compileFlags;
	}

	shared_ptr<RHI::FShader> FDX12DynamicRHI::CreateVertexShader(const std::wstring& FileName)
	{
		shared_ptr<FShader> Shader = make_shared<FDX12Shader>();
		FDX12Shader* DX12Shader = dynamic_cast<FDX12Shader*>(Shader.get());
		ID3DBlob* ErrorMsg = nullptr;
		auto HR = D3DCompileFromFile(FileName.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", GetEnableShaderDebugFlags(), 0, &DX12Shader->Shader, &ErrorMsg);
		if (FAILED(HR))
		{
			if (ErrorMsg)
			{
				OutputDebugStringA((char*)ErrorMsg->GetBufferPointer());
			}

			throw HR;
		}

		return Shader;
	}

	shared_ptr<RHI::FShader> FDX12DynamicRHI::CreatePixelShader(const std::wstring& FileName)
	{
		shared_ptr<FShader> Shader = make_shared<FDX12Shader>();
		FDX12Shader* DX12Shader = dynamic_cast<FDX12Shader*>(Shader.get());
		ID3DBlob* ErrorMsg = nullptr;
		auto HR = D3DCompileFromFile(FileName.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", GetEnableShaderDebugFlags(), 0, &DX12Shader->Shader, nullptr);
		if (FAILED(HR))
		{
			if (ErrorMsg)
			{
				OutputDebugStringA((char*)ErrorMsg->GetBufferPointer());
			}

			throw HR;
		}
		return Shader;
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
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = Initializer.SampleDesc.Count;
		return psoDesc;
	}

	ComPtr<ID3D12PipelineState> FDX12DynamicRHI::CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& PsoDesc)
	{
		ComPtr<ID3D12PipelineState> PipelineState;
		ThrowIfFailed(Device->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&PipelineState)));
		return PipelineState;
	}

	void FDX12DynamicRHI::DX12CreateConstantBuffer(FDX12CB* FDX12CB, uint32 Size)
	{
		ComPtr<ID3D12Resource>& ConstantBuffer = FDX12CB->CBRes;
		void*& VirtualAddress = FDX12CB->UploadBufferVirtualAddress; // TODO: UploadBufferVirtualAddress is nullptr
		uint32 ConstantBufferSize = Size;

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
		CreateCBVToHeaps(CbvDesc, FDX12CB);

		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		// Map: resource give cpu the right to dynamic manipulate(memcpy) it, and forbid gpu to manipulate it, until Unmap occur.
		// resource->Map(subresource, cpuReadRange, cpuVirtualAdress )
		ThrowIfFailed(ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&VirtualAddress)));
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
				uint32 adapterIndex = 0;
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
			for (uint32 adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
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

	ComPtr<ID3D12RootSignature> FDX12DynamicRHI::CreateDX12RootSig_CB0_SR1_Sa2()
	{
		ComPtr<ID3D12RootSignature> RootSignature;
		D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};
		ChooseSupportedFeatureVersion(FeatureData, D3D_ROOT_SIGNATURE_VERSION_1_1);

		CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
		CD3DX12_ROOT_PARAMETER1 rootParameters[3];

		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // frequently changed cb
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // infrequently changed srv
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); // infrequently changed sampler

		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL); // srv only used in ps
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> Signature;
		ComPtr<ID3DBlob> Error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, FeatureData.HighestVersion, &Signature, &Error));
		ThrowIfFailed(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
		return RootSignature;
	}

	void FDX12DynamicRHI::CreateGPUFence(ComPtr<ID3D12Fence>& Fence)
	{
		ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
	}

	void FDX12DynamicRHI::CreateMeshForFrameResource(FMeshActorFrameResource& MeshActorFrameResource, const FMeshActor& MeshActor)
	{
		MeshActorFrameResource.MeshToRender = CommitMeshBuffer(MeshActor);
		MeshActorFrameResource.MeshResToRender = CommitMeshResBuffer(L"shaders.hlsl", RHI::SHADER_FLAGS::CB0_SR1_Sa2);
	}

	void FDX12DynamicRHI::FrameBegin()
	{
		// reset the commandlist
		CommandLists[0].Reset();

		// common set
		CommandLists[0].CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CommandLists[0].CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[BackFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart(), BackFrameIndex, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		CommandLists[0].CommandList->ClearRenderTargetView(RtvHandle, clearColor, 0, nullptr);
		CommandLists[0].CommandList->ClearDepthStencilView(DSVHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		CommandLists[0].CommandList->ClearDepthStencilView(ShadowDepthViewHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void FDX12DynamicRHI::DrawFrame(const FFrameResource* FrameRes)
	{
		for (auto i : FrameRes->MeshActorFrameResources)
		{
			DrawMeshActorShadowPass(i);
		}
		CommandLists[0].CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DX12ShadowMap.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		for (auto i : FrameRes->MeshActorFrameResources)
		{
			DrawMeshActorBasePass(i);
		}
		CommandLists[0].CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DX12ShadowMap.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	}

	void FDX12DynamicRHI::DrawMeshActorShadowPass(const FMeshActorFrameResource& MeshActor)
	{
		FDX12Mesh* DX12Mesh = dynamic_cast<FDX12Mesh*>(MeshActor.MeshToRender.get());
		FDX12MeshRes* DX12MeshRes = dynamic_cast<FDX12MeshRes*>(MeshActor.MeshResToRender.get());
		FDX12CB* ShadowCB = dynamic_cast<FDX12CB*>(MeshActor.MeshResToRender->ShadowCB.get()); // shadow cb

		// viewport scissorect
		D3D12_VIEWPORT ShadowViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(8192), static_cast<float>(8192));
		D3D12_RECT ShadowScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(8192), static_cast<LONG>(8192));
		CommandLists[0].CommandList->RSSetViewports(1, &ShadowViewport);
		CommandLists[0].CommandList->RSSetScissorRects(1, &ShadowScissorRect);

		// set render target to nullptr, dsv to shadow texture
		CommandLists[0].CommandList->OMSetRenderTargets(0, nullptr, FALSE, &ShadowDepthViewHandle);

		// use shadow pso
		CommandLists[0].CommandList->SetPipelineState(DX12MeshRes->ShadowPSO.Get());

		// root signature
		CommandLists[0].CommandList->SetGraphicsRootSignature(DX12MeshRes->RootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { CBVSRVHeap.Get(), SamplerHeap.Get() };
		CommandLists[0].CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(0, ShadowCB->GPUHandleInHeap); // cb
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(1, NullGPUHandle); // shadow map texture
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(2, SamplerHeap->GetGPUDescriptorHandleForHeapStart()); // sampler

		// set mesh
		CommandLists[0].CommandList->IASetIndexBuffer(&DX12Mesh->IndexBufferView);
		CommandLists[0].CommandList->IASetVertexBuffers(0, 1, &DX12Mesh->VertexBufferView);
		CommandLists[0].CommandList->DrawIndexedInstanced(DX12Mesh->IndexCount, 1, 0, 0, 0);
	}

	void FDX12DynamicRHI::DrawMeshActorBasePass(const FMeshActorFrameResource& MeshActor)
	{
		FDX12Mesh* DX12Mesh = dynamic_cast<FDX12Mesh*>(MeshActor.MeshToRender.get());
		FDX12MeshRes* DX12MeshRes = dynamic_cast<FDX12MeshRes*>(MeshActor.MeshResToRender.get());
		FDX12CB* DX12CB = dynamic_cast<FDX12CB*>(MeshActor.MeshResToRender->BaseCB.get()); // base pass cb

		// viewport scissorect
		CommandLists[0].CommandList->RSSetViewports(1, &Viewport);
		CommandLists[0].CommandList->RSSetScissorRects(1, &ScissorRect);

		// rendertarget
		CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart(), BackFrameIndex, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		CommandLists[0].CommandList->OMSetRenderTargets(1, &RtvHandle, FALSE, &DSVHandle);

		// pso
		CommandLists[0].CommandList->SetPipelineState(DX12MeshRes->BasePSO.Get());

		// root signature
		CommandLists[0].CommandList->SetGraphicsRootSignature(DX12MeshRes->RootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { CBVSRVHeap.Get(), SamplerHeap.Get() };
		CommandLists[0].CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(0, DX12CB->GPUHandleInHeap); // cb
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(1, ShadowMapGPUHandleForCBVSRV); // shadow map texture
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(2, SamplerHeap->GetGPUDescriptorHandleForHeapStart()); // sampler

		// set mesh
		CommandLists[0].CommandList->IASetIndexBuffer(&DX12Mesh->IndexBufferView);
		CommandLists[0].CommandList->IASetVertexBuffers(0, 1, &DX12Mesh->VertexBufferView);
		CommandLists[0].CommandList->DrawIndexedInstanced(DX12Mesh->IndexCount, 1, 0, 0, 0);
	}

	void FDX12DynamicRHI::FrameEnd()
	{
		CommandLists[0].CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[BackFrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		// Execute the command list.
		CommandLists[0].Close();
		CommandLists[0].Execute();

		// Present the frame.
		ThrowIfFailed(RHISwapChain->Present(1, 0));
		WaitForExecuteComplete();
	}

	void FDX12DynamicRHI::WaitForExecuteComplete()
	{
		const int fence = FenceValue; //m_fenceValue: CPU fence value
		ThrowIfFailed(RHICommandQueue->Signal(Fence.Get(), fence)); // set a fence in GPU
		FenceValue++;

		if (Fence->GetCompletedValue() < fence) // if GPU run after CPU, make CPU wait for GPU
		{
			ThrowIfFailed(Fence->SetEventOnCompletion(fence, FenceEvent)); // define m_fenceEvent as the event that fire when m_fence hit the fence param
			WaitForSingleObject(FenceEvent, INFINITE); // CPU wait
		}

		BackFrameIndex = RHISwapChain->GetCurrentBackBufferIndex();
	}

	shared_ptr<RHI::FMesh> FDX12DynamicRHI::CommitMeshBuffer(const FMeshActor& MeshActor)
	{
		shared_ptr<RHI::FDX12Mesh> Mesh = make_shared<RHI::FDX12Mesh>();
		Mesh->IndexCount = static_cast<uint32>(MeshActor.MeshLODs[0].GetIndices().size());

		uint32 VertexBufferSize = static_cast<uint32>(MeshActor.MeshLODs[0].GetVertices().size() * sizeof(float));
		uint32 IndexBufferSize = static_cast<uint32>(MeshActor.MeshLODs[0].GetIndices().size() * sizeof(uint32));
		auto CommandList = CommandLists[0].CommandList;

		// vertex buffer
		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&Mesh->VertexBuffer)));

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&Mesh->VertexBufferUploadHeap)));

		NAME_D3D12_OBJECT(Mesh->VertexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = MeshActor.MeshLODs[0].GetVertices().data();
		vertexData.RowPitch = VertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources<1>(CommandList.Get(), Mesh->VertexBuffer.Get(), Mesh->VertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Mesh->VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		// Initialize the vertex buffer view.
		Mesh->VertexBufferView.BufferLocation = Mesh->VertexBuffer->GetGPUVirtualAddress();
		Mesh->VertexBufferView.StrideInBytes = MeshActor.MeshLODs[0].GetVertexStride();
		Mesh->VertexBufferView.SizeInBytes = VertexBufferSize;

		// index buffer
		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&Mesh->IndexBuffer)));

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&Mesh->IndexBufferUploadHeap)));

		NAME_D3D12_OBJECT(Mesh->IndexBuffer);

		// Copy data to the intermediate upload heap and then schedule a copy 
		// from the upload heap to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = MeshActor.MeshLODs[0].GetIndices().data();
		indexData.RowPitch = IndexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;

		UpdateSubresources<1>(CommandList.Get(), Mesh->IndexBuffer.Get(), Mesh->IndexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
		CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Mesh->IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		// Describe the index buffer view.
		Mesh->IndexBufferView.BufferLocation = Mesh->IndexBuffer->GetGPUVirtualAddress();
		Mesh->IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		Mesh->IndexBufferView.SizeInBytes = IndexBufferSize;

		return Mesh;
	}

	shared_ptr<RHI::FMeshRes> FDX12DynamicRHI::CommitMeshResBuffer(const std::wstring& FileName, const SHADER_FLAGS& flags)
	{
		shared_ptr<FMeshRes> MeshRes = make_shared<FDX12MeshRes>();
		FDX12MeshRes* DX12MeshRes = dynamic_cast<FDX12MeshRes*>(MeshRes.get());

		// 1. create pso
		WCHAR assetsPath[512];
		GetAssetsPath(assetsPath, _countof(assetsPath));
		std::wstring m_assetsPath = assetsPath + FileName;
		DX12MeshRes->VS = CreateVertexShader(m_assetsPath.c_str()); // could just use dx12 shader type otherwise FShader
		DX12MeshRes->PS = CreatePixelShader(m_assetsPath.c_str());
		shared_ptr<FPSOInitializer> initializer = make_shared<FDX12PSOInitializer>();
		InitPipeLineToMeshRes(MeshRes.get(), initializer.get(), flags);

		// 2. create cb
		DX12MeshRes->BaseCB = CreateConstantBufferToMeshRes(256);
		DX12MeshRes->ShadowCB = CreateConstantBufferToMeshRes(256);

		return MeshRes;
	}

	void FDX12DynamicRHI::CreateFenceAndEvent()
	{
		CreateGPUFence(Fence);
		FenceValue = 1;

		// Create an event handle to use for frame synchronization.
		FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (FenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}

	void FDX12DynamicRHI::BegineCreateResource()
	{
		CommandLists[0].Reset();
	}

	void FDX12DynamicRHI::EndCreateResource()
	{
		CommandLists[0].Close();
		CommandLists[0].Execute();
		WaitForExecuteComplete();
	}

	shared_ptr<RHI::FTexture> FDX12DynamicRHI::CreateEmptyTexture()
	{
		return make_shared<RHI::FDX12Texture>();
	}

	void FDX12DynamicRHI::CommitShadowMap()
{
		CD3DX12_RESOURCE_DESC shadowTexDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			static_cast<UINT>(8192), // TODO: hard coding
			static_cast<UINT>(8192), // TODO: hard coding
			1,
			1,
			DXGI_FORMAT_R32_TYPELESS,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		D3D12_CLEAR_VALUE clearValue;        // Performance tip: Tell the runtime at resource creation the desired clear value.
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		ThrowIfFailed(Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&shadowTexDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&DX12ShadowMap)));

		NAME_D3D12_OBJECT(DX12ShadowMap);
	}
}
