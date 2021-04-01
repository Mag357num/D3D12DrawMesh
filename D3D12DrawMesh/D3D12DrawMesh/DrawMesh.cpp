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
#include "DrawMesh.h"
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

void FDrawMesh::ReadCameraBinary(const string & BinFileName, XMFLOAT3 & Location, XMFLOAT3 & Dir, float & Fov, float & Aspect, XMFLOAT4 & Rotator)
{
	std::ifstream Fin(BinFileName, std::ios::binary);

	if (!Fin.is_open())
	{
		throw std::exception("open file faild.");
	}
	Fin.read((char*)&Location, sizeof(float) * 3);
	Fin.read((char*)&Dir, sizeof(float) * 3);
	Fin.read((char*)&Fov, sizeof(float));
	Fin.read((char*)&Aspect, sizeof(float));
	Fin.read((char*)&Rotator, sizeof(float) * 4);
	MainCamera.Init({ 500, 0, 0 }, {0, 0, 1}, { -1, 0, 0});
	Fin.close();
}

void FDrawMesh::ReadStaticMeshBinary(const string & BinFileName, UINT8 *& PVertData, UINT8 *& PIndtData, int & VertexBufferSize, int & VertexStride, int & IndexBufferSize)
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

void FDrawMesh::OnInit()
{
	//read cam binary
	XMFLOAT3 Location;
	XMFLOAT3 Dir;
	float Fov;
	float Aspect;
	XMFLOAT4 Rotator;
	ReadCameraBinary("SingleCameraBinary_.dat", Location, Dir, Fov, Aspect, Rotator);

	FDynamicRHI::CreateRHI();
	GDynamicRHI->RHIInit(m_useWarpDevice, BufferFrameCount, ResoWidth, ResoHeight);

	LoadPipeline();
	LoadAssets();
}

// Load the rendering pipeline dependencies.
void FDrawMesh::LoadPipeline()
{
	MainCommandAllocator = GDynamicRHI->CreateCommandAllocator();
}

// Load the sample assets.
void FDrawMesh::LoadAssets()
{
	GDynamicRHI->CreateVertexShader(GetAssetFullPath(L"shaders.hlsl").c_str());
	GDynamicRHI->CreatePixelShader(GetAssetFullPath(L"shaders.hlsl").c_str());









	GDynamicRHI->InitPipeLine();

	// Single-use command allocator and command list for creating resources.
	ComPtr<ID3D12CommandAllocator> ResourceCommitCommandAllocator = GDynamicRHI->CreateCommandAllocator();
	ComPtr<ID3D12GraphicsCommandList> ResourceCommitCommandList = GDynamicRHI->CreateCommandList(ResourceCommitCommandAllocator, nullptr);

	// Create the command list.
	MainCommandList = GDynamicRHI->CreateCommandList(MainCommandAllocator, GDynamicRHI->GetPSOArray()[0]);

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
	GDynamicRHI->UpdateVertexBuffer(ResourceCommitCommandList, GDynamicRHI->GetVertexBufferRef(), VertexBufferUploadHeap, VertexBufferSize, VertexStride, PVertData);

	// Create the index buffer.
	GDynamicRHI->UpdateIndexBuffer(ResourceCommitCommandList, GDynamicRHI->GetIndexBufferRef(), IndexBufferUploadHeap, IndexBufferSize, PIndtData);

	free(PVertData);
	free(PIndtData);

	// Create the constant buffer.
	const UINT ConstantBufferSize = sizeof(FSceneConstantBuffer); // CB size is required to be 256-byte aligned.
	GDynamicRHI->UpLoadConstantBuffer(ConstantBufferSize, ConstantBufferData, PCbvDataBegin1);

	// Close the resource creation command list and execute it to begin the vertex buffer copy into
	// the default heap.
	// encapsulated the ExecuteCommand() function.
	GDynamicRHI->CloseCommandList(ResourceCommitCommandList);
	ID3D12CommandList* PPCommandLists[] = { ResourceCommitCommandList.Get() };
	GDynamicRHI->GetCommandQueueRef()->ExecuteCommandLists(_countof(PPCommandLists), PPCommandLists);

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
	XMStoreFloat4x4(&ConstantBufferData.WorldViewProj, XMMatrixTranspose(m * v * p));

	memcpy(PCbvDataBegin1, &ConstantBufferData, sizeof(ConstantBufferData));
}

// Render the scene.
void FDrawMesh::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { MainCommandList.Get() };
	GDynamicRHI->GetCommandQueueRef()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(GDynamicRHI->GetSwapChainRef()->Present(1, 0));

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
	ThrowIfFailed(MainCommandList->Reset(MainCommandAllocator.Get(), GDynamicRHI->GetPSOArray()[0].Get()));

	// Set necessary state.
	MainCommandList->SetGraphicsRootSignature(GDynamicRHI->GetRootSignatureRef().Get());

	MainCommandList->RSSetViewports(1, &Viewport);
	MainCommandList->RSSetScissorRects(1, &ScissorRect);

	// Indicate that the back buffer will be used as a render target.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GDynamicRHI->GetRTVRef()[GDynamicRHI->GetBackBufferIndexRef()].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GDynamicRHI->GetRTVHeapRef()->GetCPUDescriptorHandleForHeapStart(), GDynamicRHI->GetBackBufferIndexRef(), GDynamicRHI->GetDeviceRef()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(GDynamicRHI->GetDSVHeapRef()->GetCPUDescriptorHandleForHeapStart());

	MainCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	MainCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	MainCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* ppHeaps[] = { GDynamicRHI->GetCBVSRVHeapRef().Get() };
	MainCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	MainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	MainCommandList->IASetIndexBuffer(&GDynamicRHI->GetIBVRef());
	MainCommandList->IASetVertexBuffers(0, 1, &GDynamicRHI->GetVBVRef());
	MainCommandList->SetGraphicsRootDescriptorTable(0, GDynamicRHI->GetCBVSRVHeapRef()->GetGPUDescriptorHandleForHeapStart());
	MainCommandList->DrawIndexedInstanced(IndexNum, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	MainCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GDynamicRHI->GetRTVRef()[GDynamicRHI->GetBackBufferIndexRef()].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
	ThrowIfFailed(GDynamicRHI->GetCommandQueueRef()->Signal(FenceGPU.Get(), fence)); // set a fence in GPU
	FenceValue++;

	// Wait until the previous frame is finished.
	// GetCompletedValue(): get the fence index that GPU still in( regard GPU in a runways with many fence )
	if (FenceGPU->GetCompletedValue() < fence) // if GPU run after CPU, make CPU wait for GPU
	{
		ThrowIfFailed(FenceGPU->SetEventOnCompletion(fence, FenceEvent)); // define m_fenceEvent as the event that fire when m_fence hit the fence param
		WaitForSingleObject(FenceEvent, INFINITE); // CPU wait
	}

	GDynamicRHI->GetBackBufferIndexRef() = GDynamicRHI->GetSwapChainRef()->GetCurrentBackBufferIndex();
}
