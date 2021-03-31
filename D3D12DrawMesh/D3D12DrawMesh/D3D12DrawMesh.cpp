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

D3D12DrawMesh::D3D12DrawMesh(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	PCbvDataBegin1(nullptr),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_constantBufferData{}
{
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

// Load the rendering pipeline dependencies.
void D3D12DrawMesh::LoadPipeline()
{
	GDynamicRHI->CreateDevice(m_useWarpDevice);
	GDynamicRHI->CreateCommandQueue();
	GDynamicRHI->CreateSwapChain(FrameCount, m_width, m_height, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_frameIndex = GDynamicRHI->GetSwapChain()->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		GDynamicRHI->CreateDescriptorHeaps(FrameCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, GDynamicRHI->GetRTVHeap());

		// Describe and create a depth stencil view (DSV) descriptor heap.
		GDynamicRHI->CreateDescriptorHeaps(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, GDynamicRHI->GetDSVHeap());

		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		GDynamicRHI->CreateDescriptorHeaps(1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, GDynamicRHI->GetCBVHeap());
	}

	// Create frame resources.
	GDynamicRHI->CreateRTVToHeaps(GDynamicRHI->GetRTVHeap(), FrameCount);

	GDynamicRHI->CreateCommandlist(MainCommandAllocator);
}

// Load the sample assets.
void D3D12DrawMesh::LoadAssets()
{
	// Create a root signature consisting of a descriptor table with a single CBV.
	{
		// choose root signature version
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		GDynamicRHI->ChooseSupportedFeatureVersion(featureData, D3D_ROOT_SIGNATURE_VERSION_1_1);

		// create root signature
		GDynamicRHI->CreateRootSignature(featureData);
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
		GraphicsPipelineStateInitializer Initializer(VertexDescription, GDynamicRHI->GetRootSignature().Get(), CD3DX12_SHADER_BYTECODE(vertexShader.Get()), CD3DX12_SHADER_BYTECODE(pixelShader.Get()),
			RasterizerStateDesc, DepthStencilDesc);
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = GDynamicRHI->CreateGraphicsPipelineStateDesc(Initializer);

		// TODO: figure out whats the different between this two way to pass a smart ptr
		//CreateGraphicsPipelineState(psoDesc, m_pipelineState);
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
	UINT8* pVertData = nullptr;
	UINT8* pIndtData = nullptr;
	int vertexBufferSize;
	int vertexStride;
	int indexBufferSize;
	ReadStaticMeshBinary("StaticMeshBinary_.dat", pVertData, pIndtData, vertexBufferSize, vertexStride, indexBufferSize);

	//Create the vertex buffer.
	// TODO: add this method to a class of RHICommandList
	GDynamicRHI->UpdateVertexBuffer(ResourceCommitCommandList, GDynamicRHI->GetVertexBuffer(), VertexBufferUploadHeap, vertexBufferSize, vertexStride, pVertData);

	// Create the index buffer.
	GDynamicRHI->UpdateIndexBuffer(ResourceCommitCommandList, GDynamicRHI->GetIndexBuffer(), IndexBufferUploadHeap, indexBufferSize, pIndtData);

	free(pVertData);
	free(pIndtData);

	// Create the constant buffer.
	const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.
	GDynamicRHI->UpdateConstantBuffer(GDynamicRHI->GetConstantBuffer(), constantBufferSize, m_constantBufferData, GDynamicRHI->GetCBVHeap(), PCbvDataBegin1);

	// Create the depth stencil view.
	GDynamicRHI->CreateDSVToHeaps(GDynamicRHI->GetDSV(), GDynamicRHI->GetDSVHeap(), m_width, m_height);

	// Close the resource creation command list and execute it to begin the vertex buffer copy into
	// the default heap.
	// encapsulated the ExecuteCommand() function.
	GDynamicRHI->CloseCommandList(ResourceCommitCommandList);
	ID3D12CommandList* ppCommandLists[] = { ResourceCommitCommandList.Get() };
	GDynamicRHI->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		GDynamicRHI->CreateGPUFence(m_fence);
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
	GDynamicRHI->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(GDynamicRHI->GetSwapChain()->Present(1, 0));

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
	MainCommandList->SetGraphicsRootSignature(GDynamicRHI->GetRootSignature().Get());

	MainCommandList->RSSetViewports(1, &m_viewport);
	MainCommandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GDynamicRHI->GetRTV()[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GDynamicRHI->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, GDynamicRHI->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(GDynamicRHI->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart());

	MainCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	MainCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	MainCommandList->ClearDepthStencilView(GDynamicRHI->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* ppHeaps[] = { GDynamicRHI->GetCBVHeap().Get() };
	MainCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	MainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	MainCommandList->IASetIndexBuffer(&GDynamicRHI->GetIBV());
	MainCommandList->IASetVertexBuffers(0, 1, &GDynamicRHI->GetVBV());
	MainCommandList->SetGraphicsRootDescriptorTable(0, GDynamicRHI->GetCBVHeap()->GetGPUDescriptorHandleForHeapStart());
	MainCommandList->DrawIndexedInstanced(m_indexNum, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GDynamicRHI->GetRTV()[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
	ThrowIfFailed(GDynamicRHI->GetCommandQueue()->Signal(m_fence.Get(), fence)); // set a fence in GPU
	m_fenceValue++;

	// Wait until the previous frame is finished.
	// GetCompletedValue(): get the fence index that GPU still in( regard GPU in a runways with many fence )
	if (m_fence->GetCompletedValue() < fence) // if GPU run after CPU, make CPU wait for GPU
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent)); // define m_fenceEvent as the event that fire when m_fence hit the fence param
		WaitForSingleObject(m_fenceEvent, INFINITE); // CPU wait
	}

	m_frameIndex = GDynamicRHI->GetSwapChain()->GetCurrentBackBufferIndex();
}
