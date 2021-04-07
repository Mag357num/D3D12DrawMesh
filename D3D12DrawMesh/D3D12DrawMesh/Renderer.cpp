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
#include "Renderer.h"

using RHI::GDynamicRHI;
using RHI::FMesh;

static RHI::FMesh* Chair = nullptr;

HWND Renderer::m_hwnd = nullptr;
FSceneConstantBuffer Renderer::ConstantBufferData = {};
UINT8* Renderer::PCbvDataBegin = nullptr;
Camera Renderer::MainCamera = Camera();
StepTimer Renderer::Timer = StepTimer();


int Renderer::Run(DXSample* pSample, HINSTANCE hInstance, int nCmdShow)
{
    // Parse the command line parameters
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    pSample->ParseCommandLineArgs(argv, argc);
    LocalFree(argv);

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXSampleClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(pSample->GetWidth()), static_cast<LONG>(pSample->GetHeight()) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        pSample->GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        pSample);

    pSample->OnInit();

	RHI::FDynamicRHI::CreateRHI();
	GDynamicRHI->RHIInit(false, 2, 1280, 720);

    MainCamera.Init({ 500, 0, 0 }, { 0, 0, 1 }, { -1, 0, 0 });
    LoadAssets(Chair, L"shaders.hlsl");

    ShowWindow(m_hwnd, nCmdShow);

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pSample->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Renderer::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_KEYDOWN:
        if (pSample)
        {
            pSample->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (pSample)
        {
            pSample->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_PAINT:
        if (pSample)
        {
			GDynamicRHI->FrameBegin();
			OnUpdate();
			RHI::GDynamicRHI->DrawMesh(Chair);

            RHI::GDynamicRHI->FrameEnd();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Renderer::LoadAssets(FMesh*& MeshPtr, std::wstring assetName)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    std::wstring m_assetsPath = assetsPath + assetName;

    // create mesh
    GDynamicRHI->CreateVertexShader(m_assetsPath.c_str());
    GDynamicRHI->CreatePixelShader(m_assetsPath.c_str());

    // create pso
    GDynamicRHI->InitPipeLine();

    // upload mesh
    MeshPtr = GDynamicRHI->CreateMesh("StaticMeshBinary_.dat");
    GDynamicRHI->UpLoadMesh(MeshPtr);

    // upload constantbuffer
    const UINT ConstantBufferSize = sizeof(FSceneConstantBuffer); // CB size is required to be 256-byte aligned.
    GDynamicRHI->UpLoadConstantBuffer(ConstantBufferSize, ConstantBufferData, PCbvDataBegin);

    GDynamicRHI->SyncFrame();
}

void Renderer::OnUpdate()
{
    Timer.Tick(NULL);

    MainCamera.Update(static_cast<float>(Timer.GetElapsedSeconds()));

    XMMATRIX m = XMMatrixTranslation(0.f, 0.f, 0.f);
    XMMATRIX v = MainCamera.GetViewMatrix();
	XMMATRIX p = MainCamera.GetProjectionMatrix(0.8f, 1280/720); // TODO: hard coding
	//XMMATRIX p = MainCamera.GetProjectionMatrix(0.8f, m_aspectRatio);

    XMStoreFloat4x4(&ConstantBufferData.WorldViewProj, XMMatrixTranspose(m * v * p));

    memcpy(PCbvDataBegin, &ConstantBufferData, sizeof(ConstantBufferData));
}