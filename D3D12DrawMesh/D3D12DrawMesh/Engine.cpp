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

#include "Engine.h"
#include "DynamicRHI.h"
#include "RenderThread.h"

using namespace Microsoft::WRL;
using RHI::GDynamicRHI;
using RHI::FMesh;
using RHI::FMeshRes;

FEngine* GEngine = nullptr;

FEngine::FEngine(uint32 width, uint32 height, std::wstring name) :
    ResoWidth(width),
    ResoHeight(height),
    Title(name),
    IsUseWarpDevice(false)
{
	GEngine = this;
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    AssetsPath = assetsPath;
    AspectRatio = static_cast<float>(width) / static_cast<float>(height);

	AssetManager = make_shared<FAssetManager>();

	CurrentScene = make_shared<FScene>();
	CurrentScene->SetCurrentCamera({ 1000, -100, 150 }, { 0, 0, 1 }, { -1, 0, 0 }, 0.8f, AspectRatio);
}

FEngine::~FEngine()
{

}

void FEngine::OnInit()
{
	// TODO: refactor here. read a bin file to load scene
	AssetManager->LoadMeshesToScene(L"Scene_.dat", CurrentScene.get());
	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start();
	FRenderThread::Get()->CreateResourceForScene(CurrentScene);
}

void FEngine::OnUpdate()
{
	CurrentScene->UpdateMainCamera(GEngine);
	FRenderThread::Get()->WaitForRenderThread();
	FRenderThread::Get()->UpdateFrameResources(CurrentScene.get());
}

void FEngine::OnRender()
{
}

void FEngine::OnDestroy()
{
	FRenderThread::DestroyRenderThread();
}

void FEngine::OnKeyDown(unsigned char Key)
{
	CurrentScene->GetCurrentCamera().OnKeyDown(Key);
}

void FEngine::OnKeyUp(unsigned char Key)
{
	CurrentScene->GetCurrentCamera().OnKeyUp(Key);
}

void FEngine::OnMouseMove(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera().OnMouseMove(x, y);
}

void FEngine::OnRightButtonDown(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera().OnRightButtonDown(x, y);
}

void FEngine::OnRightButtonUp()
{
	CurrentScene->GetCurrentCamera().OnRightButtonUp();
}
