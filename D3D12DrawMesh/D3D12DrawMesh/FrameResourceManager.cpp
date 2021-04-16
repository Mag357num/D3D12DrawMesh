#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include <gtc/matrix_transform.hpp>

void FFrameResourceManager::CreateFrameResourcesFromScene(shared_ptr<FScene> Scene, uint32 FrameCount)
{
	//new
	RHI::GDynamicRHI->BegineCreateResource();
	FrameResources.resize(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResources[FrameIndex];
		const uint32 MeshActorCount = static_cast<uint32>(Scene->MeshActors.size());
		FrameResource.MeshActorFrameResources.resize(MeshActorCount);
		for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
		{
			FMeshActorFrameResource& MeshActorFrameResource = FrameResource.MeshActorFrameResources[MeshIndex];
			MeshActorFrameResource.MeshActorResIndex = MeshIndex;
			FMeshActor& MeshActor = Scene->MeshActors[MeshIndex];
			CreateMeshActorFrameResources(MeshActorFrameResource, MeshActor); // MeshActor in Scene reflect to MeshActorFrameResource by order
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

void FFrameResourceManager::CreateMeshActorFrameResources(FMeshActorFrameResource& MeshActorFrameResource, FMeshActor& MeshActor)
{
	RHI::GDynamicRHI->CreateMeshForFrameResource(MeshActorFrameResource, MeshActor);
}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, uint32 FrameIndex)
{
	FCamera& MainCamera = Scene->GetCurrentCamera();
	FMatrix V = MainCamera.GetViewMatrix();
	FMatrix P = MainCamera.GetProjectionMatrix(1.0f, 10000.0f);
	FMatrix VPMatrix = P * V;

	FFrameResource& FrameResource = FrameResources[FrameIndex];
	const uint32 MeshActorCount = static_cast<uint32>(Scene->MeshActors.size());
	for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
	{
		const FMatrix Identity = glm::identity<FMatrix>();
		const FVector& Rotate = Scene->MeshActors[MeshIndex].Transform.Rotator;

		FMatrix RotateMatrix = glm::rotate(Identity, glm::radians(Rotate.z), FVector(0, 0, 1));
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.x), FVector(1, 0, 0));
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.y), FVector(0, 1, 0));
		FMatrix ScaleMatrix = glm::scale(Identity, Scene->MeshActors[MeshIndex].Transform.Scale);
		FMatrix TranslateMatrix = glm::translate(Identity, Scene->MeshActors[MeshIndex].Transform.Translation);

		FMatrix WorldMatrix = Identity * TranslateMatrix * RotateMatrix * ScaleMatrix; // use column matrix, multiple is right to left
		FMatrix Wvp = glm::transpose(VPMatrix * WorldMatrix);

		RHI::FCBData UpdateData;
		UpdateData.DataBuffer = reinterpret_cast<void*>(&Wvp);
		UpdateData.BufferSize = sizeof(Wvp);
		GDynamicRHI->UpdateConstantBufferInMeshRes(FrameResource.MeshActorFrameResources[MeshIndex].MeshResToRender.get(),
			&UpdateData); // MeshActor in Scene reflect to MeshActorFrameResource by order
	}
}