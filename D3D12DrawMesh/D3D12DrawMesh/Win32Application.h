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

using RHI::GDynamicRHI;
using RHI::FMesh;

class DXSample;

struct FSceneConstantBuffer : public RHI::FConstantBufferBase
{
	XMFLOAT4X4 WorldViewProj;
	float Padding[48]; // Padding so the constant buffer is 256-byte aligned.
};
static_assert((sizeof(FSceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

class Win32Application
{
public:
    static int Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void LoadAssets(FMesh*& MeshPtr, std::wstring assetName);
	static void OnUpdate();

private:
    static HWND m_hwnd;
	static FSceneConstantBuffer ConstantBufferData;
	static UINT8* PCbvDataBegin;
	static StepTimer Timer;
	static Camera MainCamera;
};
