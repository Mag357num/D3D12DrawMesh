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

using namespace Microsoft::WRL;
using RHI::GDynamicRHI;
using RHI::FMesh;
using RHI::FMeshRes;
using RHI::FActor;

Engine::Engine(UINT width, UINT height, std::wstring name) :
    ResoWidth(width),
    ResoHeight(height),
    m_title(name),
    m_useWarpDevice(false)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

Engine::~Engine()
{
}

void Engine::OnInit()
{
	// 1. init(command, swapchain, heaps)
	RHI::FDynamicRHI::CreateRHI();
	GDynamicRHI->RHIInit(false, 2, ResoWidth, ResoHeight);
	MainCamera.Init({ 500, 0, 0 }, { 0, 0, 1 }, { -1, 0, 0 });

	// 2. load res
	shared_ptr<FMesh> Mesh = GDynamicRHI->PrepareMeshData("StaticMeshBinary_.dat");
	GDynamicRHI->UpLoadMesh(Mesh.get());
	shared_ptr<FMeshRes> MeshRes = GDynamicRHI->CreateMeshRes(L"shaders.hlsl", RHI::SHADER_FLAGS::CB1_SR0);
	shared_ptr<FActor> Actor = make_shared<FActor>();
	Actor->Mesh = Mesh;
	Actor->MeshRes = MeshRes;
	Scene.Actors.push_back(Actor);
	GDynamicRHI->SyncFrame();

	// 3. start render thread
}

void Engine::OnUpdate()
{
	XMMATRIX VPMatrix = UpdateViewProj();
	for (auto i : Scene.Actors)
	{
		XMFLOAT4X4 Wvp;
		XMStoreFloat4x4(&Wvp, XMMatrixTranspose(i->MeshRes->WorldTrans * VPMatrix));
		RHI::FCBData Data;
		Data.BufferData = reinterpret_cast<void*>(&Wvp);
		Data.BufferSize = sizeof(Wvp);
		GDynamicRHI->UpdateConstantBufferInMeshRes(i->MeshRes.get(), &Data);
	}
}

void Engine::OnRender()
{
	GDynamicRHI->FrameBegin();
	GDynamicRHI->DrawScene(Scene);
	GDynamicRHI->FrameEnd();
}

void Engine::OnDestroy()
{

}

DirectX::XMMATRIX Engine::UpdateViewProj()
{
	Timer.Tick(NULL);

	MainCamera.Update(static_cast<float>(Timer.GetElapsedSeconds()));
	XMMATRIX V = MainCamera.GetViewMatrix();
	XMMATRIX P = MainCamera.GetProjectionMatrix(0.8f, float(ResoWidth)/float(ResoHeight));
	return V * P;
}

void Engine::OnKeyDown(UINT8 Key)
{
	MainCamera.OnKeyDown(Key);
}

void Engine::OnKeyUp(UINT8 Key)
{
	MainCamera.OnKeyUp(Key);
}

//// Helper function for resolving the full path of assets.
//std::wstring DXSample::GetAssetFullPath(LPCWSTR assetName)
//{
//    return m_assetsPath + assetName;
//}