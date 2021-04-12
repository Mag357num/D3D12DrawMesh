#include "FrameResourceManager.h"
#include "DynamicRHI.h"

void FFrameResourceManager::CreateRenderResourcesForScene(shared_ptr<FScene> SceneParam)
{
	// TODO: delete this block code, and load scene outside by bin file.
	shared_ptr<FMesh> Mesh = RHI::GDynamicRHI->PrepareMeshData("StaticMeshBinary_.dat");
	GDynamicRHI->UpLoadMesh(Mesh.get());
	shared_ptr<FMeshRes> MeshRes = GDynamicRHI->CreateMeshRes(L"shaders.hlsl", RHI::SHADER_FLAGS::CB1_SR0);
	shared_ptr<FActor> Actor = make_shared<FActor>();
	Actor->Mesh = Mesh;
	Actor->MeshRes = MeshRes;
	SceneParam->Actors.push_back(Actor);

	Scene = SceneParam;
}

void FFrameResourceManager::UpdateFrameResources()
{
	FCamera& MainCamera = Scene->GetCurrentCamera();
	XMMATRIX V = MainCamera.GetViewMatrix();
	XMMATRIX P = MainCamera.GetProjectionMatrix();
	XMMATRIX VPMatrix = V * P;

	for (auto i : Scene->GetActors())
	{
		XMFLOAT4X4 Wvp;
		XMStoreFloat4x4(&Wvp, XMMatrixTranspose(i->MeshRes->WorldTrans * VPMatrix));
		RHI::FCBData Data;
		Data.BufferData = reinterpret_cast<void*>(&Wvp);
		Data.BufferSize = sizeof(Wvp);
		GDynamicRHI->UpdateConstantBufferInMeshRes(i->MeshRes.get(), &Data);
	}
}
