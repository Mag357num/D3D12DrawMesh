#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include <gtc/matrix_transform.hpp>
#include "RHIResource.h"

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount)
{
	//new
	RHI::GDynamicRHI->BegineCreateResource();
	FrameResources.resize(FrameCount); // TODO: should move to init(), resize the FrameResources according to double buffering frame or triple
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResources[FrameIndex];

		const uint32 MeshActorCount = static_cast<uint32>(Scene->MeshActors.size());
		FrameResource.MeshActorFrameResources.resize(MeshActorCount);
		for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
		{
			FMeshActorFrameResource& MeshActorFrameResource = FrameResource.MeshActorFrameResources[MeshIndex];
			MeshActorFrameResource.MeshActorResIndex = MeshIndex;
			const FMeshActor& MeshActor = Scene->MeshActors[MeshIndex];
			CreateMeshActorFrameResources(MeshActorFrameResource, MeshActor); // MeshActor in Scene reflect to MeshActorFrameResource by order
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

void FFrameResourceManager::CreateMeshActorFrameResources(FMeshActorFrameResource& MeshActorFrameResource, const FMeshActor& MeshActor)
{
	RHI::GDynamicRHI->CreateMeshForFrameResource(MeshActorFrameResource, MeshActor);
}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, const uint32& FrameIndex)
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
		const FVector& Rotate = Scene->MeshActors[MeshIndex].Transform.Rotator; // x roll y pitch z yaw

		FMatrix RotateMatrix = Identity;
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.x), FVector(1, 0, 0)); // roll
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.y), FVector(0, 1, 0)); // pitch
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(Rotate.z), FVector(0, 0, 1)); // yaw

		FMatrix ScaleMatrix = glm::scale(Identity, Scene->MeshActors[MeshIndex].Transform.Scale);
		FMatrix TranslateMatrix = glm::translate(Identity, Scene->MeshActors[MeshIndex].Transform.Translation);

		FMatrix WorldMatrix = Identity * TranslateMatrix * RotateMatrix * ScaleMatrix; // use column matrix, multiple is right to left
		FMatrix WVP = glm::transpose(VPMatrix * WorldMatrix);

		FBlinnPhongCB ConstantBufferData;
		ConstantBufferData.WVP = WVP;
		ConstantBufferData.World = glm::transpose(WorldMatrix);

		FVector CamPos = Scene->SceneCamera.GetPosition();
		ConstantBufferData.CamEye = FVector4(CamPos.x, CamPos.y, CamPos.z, 1.0f);

		ConstantBufferData.Light.Dir = Scene->DirectionLight.Dir;
		ConstantBufferData.Light.Color = Scene->DirectionLight.Color;
		ConstantBufferData.Light.Intensity = Scene->DirectionLight.Intensity;

		RHI::FCBData UpdateData;
		UpdateData.DataBuffer = reinterpret_cast<void*>(&ConstantBufferData);
		UpdateData.BufferSize = sizeof(ConstantBufferData);
		GDynamicRHI->UpdateConstantBufferInMeshRes(FrameResource.MeshActorFrameResources[MeshIndex].MeshResToRender.get(),
			&UpdateData); // MeshActor in Scene reflect to MeshActorFrameResource by order
	}
}