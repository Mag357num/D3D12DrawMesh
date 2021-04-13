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
#include "Engine.h"
#include "DynamicRHI.h"
#include "RenderThread.h"

using namespace Microsoft::WRL;
using RHI::GDynamicRHI;
using RHI::FMesh;
using RHI::FMeshRes;
using RHI::FActor;

FEngine* GEngine = nullptr;

FEngine::FEngine(UINT width, UINT height, std::wstring name) :
    ResoWidth(width),
    ResoHeight(height),
    m_title(name),
    m_useWarpDevice(false)
{
	GEngine = this;
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	CurrentScene = make_shared<FScene>();
	CurrentScene->GetCurrentCamera().SetAspectRatio(m_aspectRatio);
	CurrentScene->GetCurrentCamera().SetFov(0.8f);
}

FEngine::~FEngine()
{
}

void FEngine::OnInit()
{
	// TODO: refactor here. read a bin file to load scene
	CurrentScene->GetCurrentCamera().Init({ 500, 0, 0 }, { 0, 0, 1 }, { -1, 0, 0 });
	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start();
	FRenderThread::Get()->CreateResourceForScene(CurrentScene);
}

void FEngine::OnUpdate()
{
	CurrentScene->UpdateMainCamera(GEngine);
	FRenderThread::Get()->WaitForRenderThread();
	FRenderThread::Get()->UpdateFrameResources();
}

void FEngine::OnRender()
{
	FRenderThread::Get()->RenderScene();
}

void FEngine::OnDestroy()
{
	FRenderThread::DestroyRenderThread();
}

void FEngine::OnKeyDown(UINT8 Key)
{
	CurrentScene->GetCurrentCamera().OnKeyDown(Key);
}

void FEngine::OnKeyUp(UINT8 Key)
{
	CurrentScene->GetCurrentCamera().OnKeyUp(Key);
}