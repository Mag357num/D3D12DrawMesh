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



class FDrawMesh : public DXSample
{
public:
	FDrawMesh(UINT Width, UINT Height, std::wstring Name);

	void OnInit() override;
	void OnUpdate() override {};
	void OnRender() override {};
	void OnDestroy() override;

	void ReadCameraBinary(const string& BinFileName, XMFLOAT3& Location, XMFLOAT3& Dir, float& Fov, float& Aspect, XMFLOAT4& Rotator);

private:
	static const UINT BufferFrameCount = 2;

	UINT IndexNum;
	ComPtr<ID3D12Resource> VertexBufferUploadHeap;
	ComPtr<ID3D12Resource> IndexBufferUploadHeap;

};
