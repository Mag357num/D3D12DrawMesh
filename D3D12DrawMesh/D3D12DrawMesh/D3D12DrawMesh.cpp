//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "D3D12DrawMesh.h"
#include "SimpleCamera.h"
#include <fstream>

D3D12DrawMesh::D3D12DrawMesh(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_frameIndex(0),
	PCbvDataBegin1(nullptr),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_constantBufferData{}
{
	UINT dxgiFactoryFlags = 0;
	EnableDebug(dxgiFactoryFlags);
	CreateFactory(dxgiFactoryFlags);
}

void D3D12DrawMesh::OnInit()
{
	//read cam binary
	XMFLOAT3 location;
	XMFLOAT3 dir;
	float fov;
	float aspect;
	XMFLOAT4 rotator;
	ReadCameraBinary("SingleCameraBinary_.dat", location, dir, fov, aspect, rotator);

	LoadPipeline();
	LoadAssets();
}

void D3D12DrawMesh::ReadCameraBinary(const string& binFileName, XMFLOAT3& location, XMFLOAT3& dir, float& fov, float& aspect, XMFLOAT4& rotator)
{
	std::ifstream fin(binFileName, std::ios::binary);

	if (!fin.is_open())
	{
		throw std::exception("open file faild.");
	}
	fin.read((char*)&location, sizeof(float) * 3);
	fin.read((char*)&dir, sizeof(float) * 3);
	fin.read((char*)&fov, sizeof(float));
	fin.read((char*)&aspect, sizeof(float));
	fin.read((char*)&rotator, sizeof(float) * 4);
	m_camera.Init({ 500, 0, 0 }, {0, 0, 1}, { -1, 0, 0});
	fin.close();
}

void D3D12DrawMesh::ReadStaticMeshBinary(const string& binFileName, UINT8*& pVertData, UINT8*& pIndtData, int& vertexBufferSize, int& vertexStride, int& indexBufferSize)
{
	std::ifstream fin(binFileName, std::ios::binary);

	if (!fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	fin.read((char*)&vertexStride, sizeof(int));

	fin.read((char*)&vertexBufferSize, sizeof(int));
	vertexBufferSize *= static_cast<size_t>(vertexStride);

	if (vertexBufferSize > 0)
	{
		pVertData = reinterpret_cast<UINT8*>(malloc(vertexBufferSize));
		fin.read((char*)pVertData, vertexBufferSize);
	}
	else
	{
		throw std::exception();
	}

	fin.read((char*)&indexBufferSize, sizeof(int));
	m_indexNum = indexBufferSize;
	indexBufferSize *= sizeof(int);

	if (indexBufferSize > 0)
	{
		pIndtData = reinterpret_cast<UINT8*>(malloc(indexBufferSize));
		fin.read((char*)pIndtData, indexBufferSize);
	}

	fin.close();
}

void D3D12DrawMesh::EnableDebug(UINT& DxgiFactoryFlags)
{
#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
}

void D3D12DrawMesh::CreateFactory(bool FactoryFlags)
{
	ThrowIfFailed(CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory)));
}


void D3D12DrawMesh::CreateDevice(bool HasWarpDevice)
{
	if (HasWarpDevice)
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
}

void D3D12DrawMesh::CreateCommandQueue()
{
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)));
}

void D3D12DrawMesh::CreateSwapChain(UINT FrameCount, UINT Width, UINT Height, DXGI_FORMAT Format)
{
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = Width;
	swapChainDesc.Height = Height;
	swapChainDesc.Format = Format;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(Factory->CreateSwapChainForHwnd(
		CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(Factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&SwapChain)); // convert different version of swapchain type
	m_frameIndex = SwapChain->GetCurrentBackBufferIndex();
}

void D3D12DrawMesh::CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps)
{
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	HeapDesc.NumDescriptors = NumDescriptors;
	HeapDesc.Type = Type;
	HeapDesc.Flags = Flags;
	ThrowIfFailed(Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeaps)));
}

void D3D12DrawMesh::CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE HeapsHandle(Heap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		Device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, HeapsHandle);
		HeapsHandle.Offset(1, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	}
}

void D3D12DrawMesh::CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap)
{
	Device->CreateConstantBufferView(&cbvDesc, Heap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12DrawMesh::CreateCommandlist(ComPtr<ID3D12CommandAllocator>& CommandAllocator)
{
	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));
}

// Load the rendering pipeline dependencies.
void D3D12DrawMesh::LoadPipeline()
{
	CreateDevice(m_useWarpDevice);
	CreateCommandQueue();
	CreateSwapChain(FrameCount, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM);

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		CreateDescriptorHeaps(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_rtvHeap);

		// Describe and create a depth stencil view (DSV) descriptor heap.
		CreateDescriptorHeaps(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, m_dsvHeap);

		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		CreateDescriptorHeaps(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, m_cbvHeap);
	}

	// Create frame resources.
	CreateRTVToHeaps(m_rtvHeap, FrameCount);

	CreateCommandlist(MainCommandAllocator);
}

void D3D12DrawMesh::ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version)
{
	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
}

UINT D3D12DrawMesh::GetEnableShaderDebugFlags()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	return compileFlags;
}

ComPtr<ID3DBlob> D3D12DrawMesh::CreateVertexShader(LPCWSTR FileName)
{
	ComPtr<ID3DBlob> vertexShader;
	ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", GetEnableShaderDebugFlags(), 0, &vertexShader, nullptr));
	return vertexShader;
}

ComPtr<ID3DBlob> D3D12DrawMesh::CreatePixelShader(LPCWSTR FileName)
{
	ComPtr<ID3DBlob> pixelShader;
	ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", GetEnableShaderDebugFlags(), 0, &pixelShader, nullptr));
	return pixelShader;
}

D3D12_RASTERIZER_DESC D3D12DrawMesh::CreateRasterizerStateDesc()
{
	// TODO: figure out how to make this func can create all kinds of raster state
	CD3DX12_RASTERIZER_DESC rasterizerStateDesc(D3D12_DEFAULT);
	rasterizerStateDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerStateDesc.FrontCounterClockwise = TRUE;
	return static_cast<D3D12_RASTERIZER_DESC>(rasterizerStateDesc);
}

D3D12_DEPTH_STENCIL_DESC D3D12DrawMesh::CreateDepthStencilDesc()
{
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = TRUE;
	return static_cast<D3D12_DEPTH_STENCIL_DESC>(depthStencilDesc);
}

D3D12DrawMesh::GraphicsPipelineStateInitializer::GraphicsPipelineStateInitializer(const D3D12_INPUT_LAYOUT_DESC& VertexDescription,
	ID3D12RootSignature* RootSignature, const D3D12_SHADER_BYTECODE& VS,const D3D12_SHADER_BYTECODE& PS,
	const D3D12_RASTERIZER_DESC& rasterizerStateDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc)
{
	InputLayout = VertexDescription;
	pRootSignature = RootSignature;
	this->VS = VS;
	this->PS = PS;
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

D3D12_GRAPHICS_PIPELINE_STATE_DESC D3D12DrawMesh::CreateGraphicsPipelineStateDesc(const GraphicsPipelineStateInitializer& Initializer)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = Initializer.InputLayout;
	psoDesc.pRootSignature = Initializer.pRootSignature;
	psoDesc.VS = Initializer.VS;
	psoDesc.PS = Initializer.PS;
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

void D3D12DrawMesh::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ComPtr<ID3D12PipelineState>& pipelineState)
{
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}

ComPtr<ID3D12PipelineState> D3D12DrawMesh::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
	ComPtr<ID3D12PipelineState> pipelineState;
	ThrowIfFailed(Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
	return pipelineState;
}

ComPtr<ID3D12CommandAllocator> D3D12DrawMesh::CreateCommandAllocator()
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> D3D12DrawMesh::CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
	return commandList;
}

void D3D12DrawMesh::CloseCommandList(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	ThrowIfFailed(commandList->Close());
}

D3D12_VERTEX_BUFFER_VIEW D3D12DrawMesh::UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer,
	ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData)
{
	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&VertexBuffer)));

	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&VertexBufferUploadHeap)));

	NAME_D3D12_OBJECT(VertexBuffer);

	// Copy data to the intermediate upload heap and then schedule a copy 
	// from the upload heap to the vertex buffer.
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = PVertData;
	vertexData.RowPitch = VertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources<1>(CommandList.Get(), VertexBuffer.Get(), VertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// Initialize the vertex buffer view.
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	VertexBufferView.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
	VertexBufferView.StrideInBytes = VertexStride;
	VertexBufferView.SizeInBytes = VertexBufferSize;
	return VertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW D3D12DrawMesh::UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData)
{
	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&IndexBuffer)));

	ThrowIfFailed(Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&IndexBufferUploadHeap)));

	NAME_D3D12_OBJECT(IndexBuffer);

	// Copy data to the intermediate upload heap and then schedule a copy 
	// from the upload heap to the index buffer.
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = PIndData;
	indexData.RowPitch = IndexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources<1>(CommandList.Get(), IndexBuffer.Get(), IndexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
	CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	// Describe the index buffer view.
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	IndexBufferView.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
	IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	IndexBufferView.SizeInBytes = IndexBufferSize;
	return IndexBufferView;
}

void D3D12DrawMesh::UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize, const ConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin)
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
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = ConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = ConstantBufferSize;
	CreateCBVToHeaps(cbvDesc, Heap);

	// Map and initialize the constant buffer. We don't unmap this until the
	// app closes. Keeping things mapped for the lifetime of the resource is okay.
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	// Map: resource give cpu the right to dynamic manipulate(memcpy) it, and forbid gpu to manipulate it, until Unmap occur.
	// resource->Map(subresource, cpuReadRange, cpuVirtualAdress )
	ThrowIfFailed(ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&PCbvDataBegin)));
	memcpy(PCbvDataBegin, &ConstantBufferData, ConstantBufferSize);
}

void D3D12DrawMesh::CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap)
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
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&DepthStencilBuffer)
	));

	NAME_D3D12_OBJECT(DepthStencilBuffer);

	Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &depthStencilDesc, Heap->GetCPUDescriptorHandleForHeapStart());
}

// Load the sample assets.
void D3D12DrawMesh::LoadAssets()
{
	// Create a root signature consisting of a descriptor table with a single CBV.
	{
		// choose root signature version
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		ChooseSupportedFeatureVersion(featureData, D3D_ROOT_SIGNATURE_VERSION_1_1);

		// create root signature
		{
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

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
			ThrowIfFailed(Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		}
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader = CreateVertexShader(L"shaders.hlsl");
		ComPtr<ID3DBlob> pixelShader = CreatePixelShader(L"shaders.hlsl");

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		D3D12_INPUT_LAYOUT_DESC VertexDescription = { inputElementDescs, _countof(inputElementDescs) };

		D3D12_RASTERIZER_DESC RasterizerStateDesc = CreateRasterizerStateDesc();
		D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = CreateDepthStencilDesc();

		// Describe and create the graphics pipeline state object (PSO).
		GraphicsPipelineStateInitializer Initializer(VertexDescription, m_rootSignature.Get(), CD3DX12_SHADER_BYTECODE(vertexShader.Get()), CD3DX12_SHADER_BYTECODE(pixelShader.Get()),
			RasterizerStateDesc, DepthStencilDesc);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = CreateGraphicsPipelineStateDesc(Initializer);

		// TODO: figure out whats the different between this two way to pass a smart ptr
		//CreateGraphicsPipelineState(psoDesc, m_pipelineState);
		PipelineState1 = CreateGraphicsPipelineState(PsoDesc);
	}

	// Single-use command allocator and command list for creating resources.
	ComPtr<ID3D12CommandAllocator> ResourceCommitCommandAllocator = CreateCommandAllocator();
	ComPtr<ID3D12GraphicsCommandList> ResourceCommitCommandList = CreateCommandList(ResourceCommitCommandAllocator, nullptr);

	// Create the command list.
	MainCommandList = CreateCommandList(MainCommandAllocator, PipelineState1);

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	CloseCommandList(MainCommandList);

	//read binary
	UINT8* pVertData = nullptr;
	UINT8* pIndtData = nullptr;
	int vertexBufferSize;
	int vertexStride;
	int indexBufferSize;
	ReadStaticMeshBinary("StaticMeshBinary_.dat", pVertData, pIndtData, vertexBufferSize, vertexStride, indexBufferSize);

	//Create the vertex buffer.
	// TODO: add this method to a class of RHICommandList
	m_vertexBufferView = UpdateVertexBuffer(ResourceCommitCommandList, m_vertexBuffer, VertexBufferUploadHeap, vertexBufferSize, vertexStride, pVertData);

	// Create the index buffer.
	m_indexBufferView = UpdateIndexBuffer(ResourceCommitCommandList, m_indexBuffer, IndexBufferUploadHeap, indexBufferSize, pIndtData);

	free(pVertData);
	free(pIndtData);

	// Create the constant buffer.
	const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.
	UpdateConstantBuffer(m_constantBuffer, constantBufferSize, m_constantBufferData, m_cbvHeap, PCbvDataBegin1);

	// Create the depth stencil view.
	CreateDSVToHeaps(m_depthStencil, m_dsvHeap);

	// Close the resource creation command list and execute it to begin the vertex buffer copy into
	// the default heap.
	// encapsulated the ExecuteCommand() function.
	CloseCommandList(ResourceCommitCommandList);
	ID3D12CommandList* ppCommandLists[] = { ResourceCommitCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}
}

// Update frame-based values.
void D3D12DrawMesh::OnUpdate()
{
	m_timer.Tick(NULL);


	m_camera.Update(static_cast<float>(m_timer.GetElapsedSeconds()));

	XMMATRIX m = XMMatrixTranslation(0.f, 0.f, 0.f);
	XMMATRIX v = m_camera.GetViewMatrix();
	XMMATRIX p = m_camera.GetProjectionMatrix(0.8f, m_aspectRatio);

	// Compute the model-view-projection matrix.
	XMStoreFloat4x4(&m_constantBufferData.worldViewProj, XMMatrixTranspose(m * v * p));

	memcpy(PCbvDataBegin1, &m_constantBufferData, sizeof(m_constantBufferData));
}

// Render the scene.
void D3D12DrawMesh::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { MainCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(SwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3D12DrawMesh::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void D3D12DrawMesh::OnKeyDown(UINT8 key)
{
	m_camera.OnKeyDown(key);
}

void D3D12DrawMesh::OnKeyUp(UINT8 key)
{
	m_camera.OnKeyUp(key);
}

// Fill the command list with all the render commands and dependent state.
void D3D12DrawMesh::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(MainCommandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(MainCommandList->Reset(MainCommandAllocator.Get(), PipelineState1.Get()));

	// Set necessary state.
	MainCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

	MainCommandList->RSSetViewports(1, &m_viewport);
	MainCommandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	MainCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	MainCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	MainCommandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
	MainCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	MainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	MainCommandList->IASetIndexBuffer(&m_indexBufferView);
	MainCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	MainCommandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	MainCommandList->DrawIndexedInstanced(m_indexNum, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(MainCommandList->Close());
}

void D3D12DrawMesh::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue; //m_fenceValue: CPU fence value
	ThrowIfFailed(CommandQueue->Signal(m_fence.Get(), fence)); // set a fence in GPU
	m_fenceValue++;

	// Wait until the previous frame is finished.
	// GetCompletedValue(): get the fence index that GPU still in( regard GPU in a runways with many fence )
	if (m_fence->GetCompletedValue() < fence) // if GPU run after CPU, make CPU wait for GPU
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent)); // define m_fenceEvent as the event that fire when m_fence hit the fence param
		WaitForSingleObject(m_fenceEvent, INFINITE); // CPU wait
	}

	m_frameIndex = SwapChain->GetCurrentBackBufferIndex();
}
