#include "FrameResourceManager.h"
#include "DynamicRHI.h"

void FFrameResourceManager::CreateRenderResourcesForScene(shared_ptr<FScene> SceneParam)
{
	// TODO: delete this block code, and load scene outside by bin file.
	shared_ptr<FMesh> Mesh = RHI::GDynamicRHI->PrepareMeshData(L"StaticMeshBinary_.dat");
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
	FMatrix V = MainCamera.GetViewMatrix();
	FMatrix P = MainCamera.GetProjectionMatrix();
	FMatrix VPMatrix = P * V;

	for (auto i : Scene->GetActors())
	{
		FMatrix WorldMatrix = i->MeshRes->WorldTrans;
		FMatrix Wvp = glm::transpose(VPMatrix * WorldMatrix);
		RHI::FCBData UpdateData;
		UpdateData.DataBuffer = reinterpret_cast<void*>(&Wvp);
		UpdateData.BufferSize = sizeof(Wvp);
		GDynamicRHI->UpdateConstantBufferInMeshRes(i->MeshRes.get(), &UpdateData);
	}
}
