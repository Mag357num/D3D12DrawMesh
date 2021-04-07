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

class Renderer
{
public:
	static int Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void UpdateView();
	static void OnDestroy();

private:
	static HWND m_hwnd;
	static UINT8* PCbvDataBegin;
	static StepTimer Timer;
	static Camera MainCamera;
	static XMMATRIX ViewProj;
	static UINT Width;
	static UINT Height;
	static float AspectRatio;
};
