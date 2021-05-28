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

void FEngine::Init(void* WindowHandle)
{
	HWindow = WindowHandle;

	CurrentScene = CreateScene();
	vector<TStaticMeshActor> StaticMeshActors;
	FAssetManager::Get()->LoadStaticMeshActors(L"Resource\\Scene_.dat", StaticMeshActors);
	for (auto i : StaticMeshActors)
	{
		CurrentScene->AddStaticMeshActor(i);
	}

	CurrentScene->SetCurrentCamera({ 1000.f, 0.f, 300.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, -0.2f }, 0.8f, ResoWidth, ResoHeight);

	shared_ptr<ACharacter> Cha = FAssetManager::Get()->CreateCharacter();
	shared_ptr<TSkeletalMeshComponent> SkeMeshCom = FAssetManager::Get()->CreateSkeletalMeshComponent();
	shared_ptr<TSkeletalMesh> SkeMesh = FAssetManager::Get()->CreateSkeletalMesh(L"Resource\\SkeletalMeshBinary_.dat");
	shared_ptr<FSkeleton> Ske = FAssetManager::Get()->CreateSkeleton(L"Resource\\SkeletonBinary_.dat");
	shared_ptr<FAnimSequence> Seq_Run = FAssetManager::Get()->CreateAnimSequence(L"Resource\\SequenceRun_.dat");
	shared_ptr<FAnimSequence> Seq_Idle = FAssetManager::Get()->CreateAnimSequence(L"Resource\\SequenceIdle_.dat");
	Seq_Run->SetSkeleton(Ske.get());
	Seq_Idle->SetSkeleton(Ske.get());
	SkeMesh->SetSkeleton(Ske);

	SkeMeshCom->InitAnimation();
	SkeMeshCom->AddSequence(std::pair<string, shared_ptr<FAnimSequence>>("Run", Seq_Run));
	SkeMeshCom->AddSequence(std::pair<string, shared_ptr<FAnimSequence>>("Idle", Seq_Idle));
	SkeMeshCom->SetSkeletalMesh(SkeMesh);

	SkeMeshCom->SetTransform({ { 1.f, 1.f, 1.f }, FQuat(EulerToQuat(FEuler(0.f, 0.f, 0.f))), { 300.f, 200.f, 0.f } });

	Cha->SetSkeletalMeshCom(SkeMeshCom);
	CurrentScene->SetCharacter(Cha);

	// thread
	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start();
	FRenderThread::Get()->CreateFrameResource(CurrentScene);
}

void FEngine::Tick()
{
	FRenderThread::Get()->WaitForRenderThread();

	Timer.Tick(NULL);
	CurrentScene->Tick(Timer); // all actors store in FScene for now

	// TODO: remove FrameRes concept add into the tick layer, update static mesh's constant buffer when tick them
	FRenderThread::Get()->UpdateFrameRes(CurrentScene.get());
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
	CurrentScene->GetCharacter()->OnKeyDown(Key);
}

void FEngine::OnKeyUp(unsigned char Key)
{
	CurrentScene->GetCurrentCamera().OnKeyUp(Key);
	CurrentScene->GetCharacter()->OnKeyUp(Key);
}

void FEngine::OnMouseMove(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera().OnMouseMove(x, y);
	CurrentScene->GetCharacter()->OnMouseMove(x, y);
}

void FEngine::OnButtonDown(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera().OnButtonDown(x, y);
	CurrentScene->GetCharacter()->OnButtonDown(x, y);
}

void FEngine::OnButtonUp()
{
	CurrentScene->GetCurrentCamera().OnButtonUp();
	CurrentScene->GetCharacter()->OnButtonUp();
}

void FEngine::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;
	std::wstring mMainWndCaption = L"d3d App";

	frameCnt++;

	// Compute averages over one second period.
	if ((Timer.GetTotalSeconds() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		wstring fpsStr = std::to_wstring(fps);
		wstring mspfStr = std::to_wstring(mspf);

		wstring windowText = mMainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText((HWND)HWindow, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}
