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

#pragma once

#include "DXSample.h"
#include "DirectXMath.h"
#include "SimpleCamera.h"
#include "StepTimer.h"
#include "string.h"

using std::string;
using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

class D3D12DrawMesh : public DXSample
{
public:
	D3D12DrawMesh(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
	virtual void OnKeyDown(UINT8 key);
	virtual void OnKeyUp(UINT8 key);
	void ReadCameraBinary(const string& binFileName, XMFLOAT3& location, XMFLOAT3& target, float& fov, float& aspect, XMFLOAT4& rotator);
	void ReadStaticMeshBinary(const string& binFileName, UINT8*& pVertData, UINT8*& pIndtData, int& vertexBufferSize, int& vertexStride, int& indexBufferSize);

private:
	static const UINT FrameCount = 2;

	struct ConstantBufferBase
	{
	};

	struct SceneConstantBuffer : public ConstantBufferBase
	{
		XMFLOAT4X4 worldViewProj;
		float padding[48]; // Padding so the constant buffer is 256-byte aligned.
	};
	static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
	ComPtr<ID3D12PipelineState> PipelineState1;
	UINT m_cbSrvDescriptorSize;
	ComPtr<ID3D12Resource> m_depthStencil;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	ComPtr<ID3D12Resource> m_constantBuffer;
	SceneConstantBuffer m_constantBufferData;
	UINT8* PCbvDataBegin1;
	StepTimer m_timer;
	Camera m_camera;
	UINT m_indexNum;
	ComPtr<ID3D12Resource> VertexBufferUploadHeap;
	ComPtr<ID3D12Resource> IndexBufferUploadHeap;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();

	// RHI
public:
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

	void EnableDebug(UINT& DxgiFactoryFlags);
	void CreateFactory(bool FactoryFlags);
	void CreateDevice(bool HasWarpDevice);

	void CreateCommandQueue();
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, const ComPtr<ID3D12PipelineState>& PipelineState);
	void CloseCommandList(ComPtr<ID3D12GraphicsCommandList> commandList);
	D3D12_VERTEX_BUFFER_VIEW UpdateVertexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& VertexBuffer, ComPtr<ID3D12Resource>& VertexBufferUploadHeap, UINT VertexBufferSize, UINT VertexStride, UINT8* PVertData);
	D3D12_INDEX_BUFFER_VIEW UpdateIndexBuffer(ComPtr<ID3D12GraphicsCommandList> CommandList, ComPtr<ID3D12Resource>& IndexBuffer, ComPtr<ID3D12Resource>& IndexBufferUploadHeap, UINT IndexBufferSize, UINT8* PIndData);
	void ExecuteCommand(ID3D12CommandList* ppCommandLists){ }; // TODO: finish this


	void CreateSwapChain(UINT FrameCount, UINT Width, UINT Height, DXGI_FORMAT Format);
	void CreateDescriptorHeaps(const UINT& NumDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const D3D12_DESCRIPTOR_HEAP_FLAGS& Flags, ComPtr<ID3D12DescriptorHeap>& DescriptorHeaps);
	void CreateRTVToHeaps(ComPtr<ID3D12DescriptorHeap>& Heap, const UINT& FrameCount);
	void CreateCBVToHeaps(const D3D12_CONSTANT_BUFFER_VIEW_DESC& cbvDesc, ComPtr<ID3D12DescriptorHeap>& Heap);
	void CreateDSVToHeaps(ComPtr<ID3D12Resource>& DepthStencilBuffer, ComPtr<ID3D12DescriptorHeap>& Heap);
	void CreateCommandlist(ComPtr<ID3D12CommandAllocator>& CommandAllocator);
	void ChooseSupportedFeatureVersion(D3D12_FEATURE_DATA_ROOT_SIGNATURE& featureData, const D3D_ROOT_SIGNATURE_VERSION& Version);
	UINT GetEnableShaderDebugFlags();
	ComPtr<ID3DBlob> CreateVertexShader(LPCWSTR FileName);
	ComPtr<ID3DBlob> CreatePixelShader(LPCWSTR FileName);
	D3D12_RASTERIZER_DESC CreateRasterizerStateDesc();
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC CreateGraphicsPipelineStateDesc(const GraphicsPipelineStateInitializer& Initializer);
	void CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc, ComPtr<ID3D12PipelineState>& pipelineState);
	ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);
	void UpdateConstantBuffer(ComPtr<ID3D12Resource>& ConstantBuffer, const UINT& ConstantBufferSize, const ConstantBufferBase& ConstantBufferData, ComPtr<ID3D12DescriptorHeap>& Heap, UINT8*& PCbvDataBegin);
	void CreateRootSignature(D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData);

public:
	ComPtr<ID3D12Device>				Device;
	ComPtr<IDXGIFactory4>				Factory;
	ComPtr<IDXGISwapChain3>				SwapChain;
	ComPtr<ID3D12GraphicsCommandList>	MainCommandList;
	ComPtr<ID3D12CommandQueue>			CommandQueue;
	ComPtr<ID3D12CommandAllocator>		MainCommandAllocator;
};
