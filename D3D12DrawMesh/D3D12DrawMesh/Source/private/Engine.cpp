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
	CurrentScene->SetCurrentCamera({ 1000.f, 0.f, 300.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, -0.2f }, 0.8f, ResoWidth, ResoHeight); // TODO: hard code

	// init a character to scene // TODO: this logic should not be engine's work
	shared_ptr<ACharacter> Cha = FAssetManager::Get()->CreateCharacter();
	shared_ptr<FSkeletalMeshComponent> SkeMeshCom = FAssetManager::Get()->CreateSkeletalMeshComponent();
	shared_ptr<FSkeletalMesh> SkeMesh = FAssetManager::Get()->CreateSkeletalMesh(L"SkeletalMeshBinary_.dat"); // TODO: hard code
	shared_ptr<FSkeleton> Ske = FAssetManager::Get()->CreateSkeleton(L"SkeletonBinary_.dat"); // TODO: hard code
	shared_ptr<FAnimSequence> Seq_Run = FAssetManager::Get()->CreateAnimSequence(L"SequenceRun_.dat");
	shared_ptr<FAnimSequence> Seq_Idle = FAssetManager::Get()->CreateAnimSequence(L"SequenceIdle_.dat");
	Seq_Run->SetSkeleton(Ske.get());
	Seq_Idle->SetSkeleton(Ske.get());
	SkeMesh->SetSkeleton(Ske);
	SkeMeshCom->InitAnimation();
	SkeMeshCom->AddSequence(std::pair<string, shared_ptr<FAnimSequence>>("Run", Seq_Run));
	SkeMeshCom->AddSequence(std::pair<string, shared_ptr<FAnimSequence>>("Idle", Seq_Idle));
	SkeMeshCom->SetSkeletalMesh(SkeMesh);

	SkeMeshCom->SetTransform({ { 1.f, 1.f, 1.f }, FQuat(EulerToQuat(FEuler(0.f, 0.f, 0.f))), { 300.f, 200.f, 0.f } });
	SkeMeshCom->SetShaderFileName(L"Shadow_SceneColor_SkeletalMesh.hlsl");

	Cha->SetSkeletalMeshCom(SkeMeshCom);
	CurrentScene->SetCharacter(Cha);

	// thread
	FRenderThread::CreateRenderThread();
	FRenderThread::Get()->Start(); // TODO: Start() will make DoRender invoked after every "one" task, which is not so right
	FRenderThread::Get()->CreateFrameResource(CurrentScene);
}

void FEngine::Tick()
{
	FRenderThread::Get()->WaitForRenderThread();

	Timer.Tick(NULL);
	CurrentScene->Tick(Timer); // all actors store in FScene for now

	// TODO: remove FrameRes concept add into the tick layer out, update static mesh's constant buffer when tick them
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

void FEngine::OnRightButtonDown(uint32 x, uint32 y)
{
	CurrentScene->GetCurrentCamera().OnRightButtonDown(x, y);
	CurrentScene->GetCharacter()->OnRightButtonDown(x, y);
}

void FEngine::OnRightButtonUp()
{
	CurrentScene->GetCurrentCamera().OnRightButtonUp();
	CurrentScene->GetCharacter()->OnRightButtonUp();
}