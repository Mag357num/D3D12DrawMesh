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
#include "DynamicRHI.h"

using namespace RHI;
using RHI::GDynamicRHI;

FDrawMesh::FDrawMesh(UINT Width, UINT Height, std::wstring Name) :
	DXSample(Width, Height, Name),
	PCbvDataBegin1(nullptr),
	Viewport(0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height)),
	ScissorRect(0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height)),
	ConstantBufferData{}
{
}

void FDrawMesh::OnInit()
{
	//read cam binary
	XMFLOAT3 Location;
	XMFLOAT3 Dir;
	float Fov;
	float Aspect;
	XMFLOAT4 Rotator;
	ReadCameraBinary("SingleCameraBinary_.dat", Location, Dir, Fov, Aspect, Rotator);

	LoadPipeline();
	LoadAssets();
}

void FDrawMesh::ReadCameraBinary(const string & BinFileName, XMFLOAT3 & Location, XMFLOAT3 & Dir, float & Fov, float & Aspect, XMFLOAT4 & Rotator)
{
	std::ifstream fin(BinFileName, std::ios::binary);

	if (!fin.is_open())
	{
		throw std::exception("open file faild.");
	}
	fin.read((char*)&Location, sizeof(float) * 3);
	fin.read((char*)&Dir, sizeof(float) * 3);
	fin.read((char*)&Fov, sizeof(float));
	fin.read((char*)&Aspect, sizeof(float));
	fin.read((char*)&Rotator, sizeof(float) * 4);
	MainCamera.Init({ 500, 0, 0 }, {0, 0, 1}, { -1, 0, 0});
	fin.close();
}

void FDrawMesh::ReadStaticMeshBinary(const string & BinFileName, UINT8 *& PVertData, UINT8 *& PIndtData, int & VertexBufferSize, int & VertexStride, int & IndexBufferSize)
{
	std::ifstream fin(BinFileName, std::ios::binary);

	if (!fin.is_open())
	{
		throw std::exception("open file faild.");
	}

	fin.read((char*)&VertexStride, sizeof(int));

	fin.read((char*)&VertexBufferSize, sizeof(int));
	VertexBufferSize *= static_cast<size_t>(VertexStride);

	if (VertexBufferSize > 0)
	{
		PVertData = reinterpret_cast<UINT8*>(malloc(VertexBufferSize));
		fin.read((char*)PVertData, VertexBufferSize);
	}
	else
	{
		throw std::exception();
	}

	fin.read((char*)&IndexBufferSize, sizeof(int));
	IndexNum = IndexBufferSize;
	IndexBufferSize *= sizeof(int);

	if (IndexBufferSize > 0)
	{
		PIndtData = reinterpret_cast<UINT8*>(malloc(IndexBufferSize));
		fin.read((char*)PIndtData, IndexBufferSize);
	}

	fin.close();
}

// Load the rendering pipeline dependencies.
void FDrawMesh::LoadPipeline()
{
	GDynamicRHI->CreateDevice(m_useWarpDevice);
	GDynamicRHI->CreateCommandQueue();
	GDynamicRHI->CreateSwapChain(FrameCount, ResoWidth, ResoHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

	FrameIndex = GDynamicRHI->GetSwapChain()->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		GDynamicRHI->CreateDescriptorHeaps(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, GDynamicRHI->GetRTVHeap());

		// Describe and create a depth stencil view (DSV) descriptor heap.
		GDynamicRHI->CreateDescriptorHeaps(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, GDynamicRHI->GetDSVHeap());

		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		GDynamicRHI->CreateDescriptorHeaps(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, GDynamicRHI->GetCBVSRVHeap());
	}

	// Create frame resources.
	GDynamicRHI->CreateRTVToHeaps(GDynamicRHI->GetRTVHeap(), FrameCount);

	GDynamicRHI->CreateCommandlist(MainCommandAllocator);
}

// Load the sample assets.
void FDrawMesh::LoadAssets()
{
	// Create a root signature consisting of a descriptor table with a single CBV.
	{
		// choose root signature version
		D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};
		GDynamicRHI->ChooseSupportedFeatureVersion(FeatureData, D3D_ROOT_SIGNATURE_VERSION_1_1);

		// create root signature
		GDynamicRHI->CreateRootSignature(FeatureData);
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader = GDynamicRHI->CreateVertexShader(GetAssetFullPath(L"shaders.hlsl").c_str());
		ComPtr<ID3DBlob> pixelShader = GDynamicRHI->CreatePixelShader(GetAssetFullPath(L"shaders.hlsl").c_str());

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

		D3D12_RASTERIZER_DESC RasterizerStateDesc = GDynamicRHI->CreateRasterizerStateDesc();
		D3D12_DEPTH_STENCIL_DESC DepthStencilDesc = GDynamicRHI->CreateDepthStencilDesc();

		// Describe and create the graphics pipeline state object (PSO).
		FGraphicsPipelineStateInitializer Initializer(VertexDescription, GDynamicRHI->GetRootSignature().Get(), CD3DX12_SHADER_BYTECODE(vertexShader.Get()), CD3DX12_SHADER_BYTECODE(pixelShader.Get()),
			RasterizerStateDesc, DepthStencilDesc);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = GDynamicRHI->CreateGraphicsPipelineStateDesc(Initializer);

		// TODO: figure out whats the different between this two way to pass a smart ptr
		PipelineState1 = GDynamicRHI->CreateGraphicsPipelineState(PsoDesc);
	}

	// Single-use command allocator and command list for creating resources.
	ComPtr<ID3D12CommandAllocator> ResourceCommitCommandAllocator = GDynamicRHI->CreateCommandAllocator();
	ComPtr<ID3D12GraphicsCommandList> ResourceCommitCommandList = GDynamicRHI->CreateCommandList(ResourceCommitCommandAllocator, nullptr);

	// Create the command list.
	MainCommandList = GDynamicRHI->CreateCommandList(MainCommandAllocator, PipelineState1);

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	GDynamicRHI->CloseCommandList(MainCommandList);

	//read binary
	UINT8* PVertData = nullptr;
	UINT8* PIndtData = nullptr;
	int VertexBufferSize;
	int VertexStride;
	int IndexBufferSize;
	ReadStaticMeshBinary("StaticMeshBinary_.dat", PVertData, PIndtData, VertexBufferSize, VertexStride, IndexBufferSize);

	//Create the vertex buffer.
	// TODO: add this method to a class of RHICommandList
	GDynamicRHI->UpdateVertexBuffer(ResourceCommitCommandList, GDynamicRHI->GetVertexBuffer(), VertexBufferUploadHeap, VertexBufferSize, VertexStride, PVertData);

	// Create the index buffer.
	GDynamicRHI->UpdateIndexBuffer(ResourceCommitCommandList, GDynamicRHI->GetIndexBuffer(), IndexBufferUploadHeap, IndexBufferSize, PIndtData);

	free(PVertData);
	free(PIndtData);

	// Create the constant buffer.
	const UINT ConstantBufferSize = sizeof(FSceneConstantBuffer);    // CB size is required to be 256-byte aligned.
	GDynamicRHI->UpdateConstantBuffer(GDynamicRHI->GetConstantBuffer(), ConstantBufferSize, ConstantBufferData, GDynamicRHI->GetCBVSRVHeap(), PCbvDataBegin1);

	// Create the depth stencil view.
	GDynamicRHI->CreateDSVToHeaps(GDynamicRHI->GetDSV(), GDynamicRHI->GetDSVHeap(), ResoWidth, ResoHeight);

	// Close the resource creation command list and execute it to begin the vertex buffer copy into
	// the default heap.
	// encapsulated the ExecuteCommand() function.
	GDynamicRHI->CloseCommandList(ResourceCommitCommandList);
	ID3D12CommandList* PPCommandLists[] = { ResourceCommitCommandList.Get() };
	GDynamicRHI->GetCommandQueue()->ExecuteCommandLists(_countof(PPCommandLists), PPCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		GDynamicRHI->CreateGPUFence(FenceGPU);
		FenceValue = 1;

		// Create an event handle to use for frame synchronization.
		FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (FenceEvent == nullptr)
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
void FDrawMesh::OnUpdate()
{
	Timer.Tick(NULL);


	MainCamera.Update(static_cast<float>(Timer.GetElapsedSeconds()));

	XMMATRIX m = XMMatrixTranslation(0.f, 0.f, 0.f);
	XMMATRIX v = MainCamera.GetViewMatrix();
	XMMATRIX p = MainCamera.GetProjectionMatrix(0.8f, m_aspectRatio);

	// Compute the model-view-projection matrix.
	XMStoreFloat4x4(&ConstantBufferData.worldViewProj, XMMatrixTranspose(m * v * p));

	memcpy(PCbvDataBegin1, &ConstantBufferData, sizeof(ConstantBufferData));
}

// Render the scene.
void FDrawMesh::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { MainCommandList.Get() };
	GDynamicRHI->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(GDynamicRHI->GetSwapChain()->Present(1, 0));

	WaitForPreviousFrame();
}

void FDrawMesh::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(FenceEvent);
}

void FDrawMesh::OnKeyDown(UINT8 Key)
{
	MainCamera.OnKeyDown(Key);
}

void FDrawMesh::OnKeyUp(UINT8 Key)
{
	MainCamera.OnKeyUp(Key);
}

// Fill the command list with all the render commands and dependent state.
void FDrawMesh::PopulateCommandList()
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
	MainCommandList->SetGraphicsRootSignature(GDynamicRHI->GetRootSignature().Get());

	MainCommandList->RSSetViewports(1, &Viewport);
	MainCommandList->RSSetScissorRects(1, &ScissorRect);

	// Indicate that the back buffer will be used as a render target.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GDynamicRHI->GetRTV()[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GDynamicRHI->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), FrameIndex, GDynamicRHI->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(GDynamicRHI->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());

	MainCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	MainCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	MainCommandList->ClearDepthStencilView(GDynamicRHI->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* ppHeaps[] = { GDynamicRHI->GetCBVSRVHeap().Get() };
	MainCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	MainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	MainCommandList->IASetIndexBuffer(&GDynamicRHI->GetIBV());
	MainCommandList->IASetVertexBuffers(0, 1, &GDynamicRHI->GetVBV());
	MainCommandList->SetGraphicsRootDescriptorTable(0, GDynamicRHI->GetCBVSRVHeap()->GetGPUDescriptorHandleForHeapStart());
	MainCommandList->DrawIndexedInstanced(IndexNum, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GDynamicRHI->GetRTV()[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(MainCommandList->Close());
}

void FDrawMesh::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = FenceValue; //m_fenceValue: CPU fence value
	ThrowIfFailed(GDynamicRHI->GetCommandQueue()->Signal(FenceGPU.Get(), fence)); // set a fence in GPU
	FenceValue++;

	// Wait until the previous frame is finished.
	// GetCompletedValue(): get the fence index that GPU still in( regard GPU in a runways with many fence )
	if (FenceGPU->GetCompletedValue() < fence) // if GPU run after CPU, make CPU wait for GPU
	{
		ThrowIfFailed(FenceGPU->SetEventOnCompletion(fence, FenceEvent)); // define m_fenceEvent as the event that fire when m_fence hit the fence param
		WaitForSingleObject(FenceEvent, INFINITE); // CPU wait
	}

	FrameIndex = GDynamicRHI->GetSwapChain()->GetCurrentBackBufferIndex();
}
