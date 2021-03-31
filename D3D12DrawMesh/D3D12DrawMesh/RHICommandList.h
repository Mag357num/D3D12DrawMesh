#pragma once
#include "stdafx.h"
#include "Win32Application.h"
#include "DynamicRHI.h"

using namespace RHI;

class FRHICommandListImmediate
{
public:
	FRHICommandListImmediate();
	~FRHICommandListImmediate();

	//void EnableDebug(UINT& DxgiFactoryFlags) { GDynamicRHI->EnableDebug(DxgiFactoryFlags); }
	//void CreateFactory(bool FactoryFlags) { GDynamicRHI->CreateFactory(FactoryFlags); }
	//void CreateDevice(bool HasWarpDevice) { GDynamicRHI->CreateDevice(HasWarpDevice); }

	//void CreateCommandQueue() { GDynamicRHI->CreateCommandQueue(); }
	//void CreateCommandlist(ComPtr<ID3D12CommandAllocator>& CommandAllocator) { GDynamicRHI->CreateCommandlist(CommandAllocator); }
	//ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() { GDynamicRHI->CreateCommandAllocator(); }
	//ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState) { GDynamicRHI->CreateCommandList(commandAllocator, PipelineState); }
	//void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> commandList) { GDynamicRHI->CloseCommandList(commandList); }
	//D3D12_VERTEX_BUFFER_VIEW UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer, ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData) { GDynamicRHI->UpdateVertexBuffer(CommandList, VertexBuffer, VertexBufferUploadHeap, VertexBufferSize, VertexStride, PVertData); }
	//D3D12_INDEX_BUFFER_VIEW UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData) { GDynamicRHI->UpdateIndexBuffer(CommandList, IndexBuffer, IndexBufferUploadHeap, IndexBufferSize, PIndData); }
	//void ExecuteCommand(ID3D12CommandList* ppCommandLists) { GDynamicRHI->ExecuteCommand(ppCommandLists); }

	//void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height, DXGI_FORMAT Format) { GDynamicRHI->CreateSwapChain(FrameCount, Width, Height, Format); }
	//void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps) { GDynamicRHI->CreateDescriptorHeaps(NumDescriptors, Type, Flags, DescriptorHeaps); }
	//void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount) { GDynamicRHI->CreateRTVToHeaps(Heap, FrameCount); }
	//void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap) { GDynamicRHI->CreateCBVToHeaps(cbvDesc, Heap); }
	//void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap, UINT Width, UINT Height) { GDynamicRHI->CreateDSVToHeaps(DepthStencilBuffer, Heap, Width, Height); }
	//void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version) { GDynamicRHI->ChooseSupportedFeatureVersion(featureData, Version); }

};
