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
using RHI::FScene;
using RHI::FActor;
using RHI::FMeshRes;

static RHI::FScene Scene = FScene();

HWND Renderer::m_hwnd = nullptr;
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



    // 1. init(command, swapchain, heaps)
	RHI::FDynamicRHI::CreateRHI();
	GDynamicRHI->RHIInit(false, 2, 1280, 720);
    MainCamera.Init({ 500, 0, 0 }, { 0, 0, 1 }, { -1, 0, 0 });

    // 2. load scene
    Scene;

    // 3. create actor( mesh + mesh resource )
	FMesh* Mesh = GDynamicRHI->CreateMesh("StaticMeshBinary_.dat");
    GDynamicRHI->UpLoadMesh(Mesh);
    FMeshRes* MeshRes = GDynamicRHI->CreateMeshRes(L"shaders.hlsl", RHI::SHADER_FLAGS::CB1_SR0);
	FActor Actor;
	Actor.Mesh = Mesh;
	Actor.MeshRes = MeshRes;
	Scene.Actors.push_back(Actor);
	GDynamicRHI->SyncFrame();

    // 4. draw scene
    // code below





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
			//OnUpdate();
			RHI::GDynamicRHI->DrawScene(Scene);

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

void Renderer::OnUpdate()
{
 //   Timer.Tick(NULL);

 //   MainCamera.Update(static_cast<float>(Timer.GetElapsedSeconds()));

 //   XMMATRIX m = XMMatrixTranslation(0.f, 0.f, 0.f);
 //   XMMATRIX v = MainCamera.GetViewMatrix();
	//XMMATRIX p = MainCamera.GetProjectionMatrix(0.8f, 1280/720); // TODO: hard coding
	////XMMATRIX p = MainCamera.GetProjectionMatrix(0.8f, m_aspectRatio);

 //   XMStoreFloat4x4(&ConstantBufferData.WorldViewProj, XMMatrixTranspose(m * v * p));

 //   memcpy(PCbvDataBegin, &ConstantBufferData, sizeof(ConstantBufferData));
}