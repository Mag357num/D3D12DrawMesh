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
#include "DynamicRHI.h"

using std::string;
using namespace DirectX;
using namespace RHI;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

struct SceneConstantBuffer : public ConstantBufferBase
{
	XMFLOAT4X4 worldViewProj;
	float padding[48]; // Padding so the constant buffer is 256-byte aligned.
};
static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");


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

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	StepTimer m_timer;
	Camera m_camera;
	UINT8* PCbvDataBegin1;
	SceneConstantBuffer m_constantBufferData;
	UINT m_indexNum;
	ComPtr<ID3D12PipelineState> PipelineState1;
	ComPtr<ID3D12Resource> VertexBufferUploadHeap;
	ComPtr<ID3D12Resource> IndexBufferUploadHeap;

	ComPtr<ID3D12GraphicsCommandList> MainCommandList;
	ComPtr<ID3D12CommandAllocator> MainCommandAllocator;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;
	ComPtr<ID3D12Fence> m_fence;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};
