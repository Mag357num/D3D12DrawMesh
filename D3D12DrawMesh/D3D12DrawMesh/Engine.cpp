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
	AspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

FEngine::~FEngine()
{
}

void FEngine::Init()
{
	CurrentScene = FAssetManager::Get()->LoadStaticMeshActorsCreateScene(L"Scene_.dat");
	CurrentScene->SetCurrentCamera({ -600.f, 800.f, 100.f }, { 0.f, 0.f, 1.f }, { 1.f, -1.f, 0.2f }, 0.8f, AspectRatio);

	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start(); // TODO: Start() will make DoRender invoked after every "one" task, which is not so right
	FRenderThread::Get()->CreateFrameResource(CurrentScene);
}

void FEngine::Tick()
{
	FRenderThread::Get()->WaitForRenderThread();

	// tick
	Timer.Tick(NULL);
	CurrentScene->Tick(Timer); // all actors store in FScene for now

	FRenderThread::Get()->TickFrameRes(CurrentScene.get());
}

void FEngine::Render()
{
}

void FEngine::Destroy()
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