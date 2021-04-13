#include "FrameResourceManager.h"
#include "DynamicRHI.h"

// TODO: make this class platform no dependent

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
		FMatrix W = i->MeshRes->WorldTrans;
		XMMATRIX WorldMatrix
		(
			W.Row0[0], W.Row0[1], W.Row0[2], W.Row0[3],
			W.Row1[0], W.Row1[1], W.Row1[2], W.Row1[3],
			W.Row2[0], W.Row2[1], W.Row2[2], W.Row2[3],
			W.Row3[0], W.Row3[1], W.Row3[2], W.Row3[3]
		);
		XMStoreFloat4x4(&Wvp, XMMatrixTranspose(WorldMatrix * VPMatrix));
		RHI::FCBData UpdateData;
		UpdateData.DataBuffer = reinterpret_cast<void*>(&Wvp);
		UpdateData.BufferSize = sizeof(Wvp);
		GDynamicRHI->UpdateConstantBufferInMeshRes(i->MeshRes.get(), &UpdateData);
	}
}
