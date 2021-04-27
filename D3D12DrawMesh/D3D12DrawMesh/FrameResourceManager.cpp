#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include <gtc/matrix_transform.hpp>
#include "RHIResource.h"
#include "DX12Resource.h"

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount)
{
	//new
	RHI::GDynamicRHI->BegineCreateResource();
	FrameResources.resize(FrameCount); // TODO: should move to init(), resize the FrameResources according to double buffering frame or triple
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResources[FrameIndex];

		// create mesh resource
		const uint32 MeshActorCount = static_cast<uint32>(Scene->MeshActors.size());
		FrameResource.MeshActorFrameResources.resize(MeshActorCount);
		for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
		{
			FMeshActorFrameResource& MeshActorFrameResource = FrameResource.MeshActorFrameResources[MeshIndex];
			MeshActorFrameResource.MeshActorResIndex = MeshIndex;
			const FMeshActor& MeshActor = Scene->MeshActors[MeshIndex];
			CreateMeshActorFrameResources(MeshActorFrameResource, MeshActor); // MeshActor in Scene reflect to MeshActorFrameResource by order
		}

		// create shadow map
		FrameResource.ShadowMap = GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP);

		// create Ds map
		FrameResource.DepthStencilMap = GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP);

		// create sampler
		FrameResource.ClampSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP);

		// create rendertarget
		for (uint32 i = 0; i < 3; i++)
		{
			FrameResource.RenderTargets[i] = GDynamicRHI->CreateAndCommitRenderTarget(i);
		}

		// commit maps
		GDynamicRHI->CommitTextureAsView(FrameResource.ShadowMap.get(), FViewType::Dsv);
		GDynamicRHI->CommitTextureAsView(FrameResource.ShadowMap.get(), FViewType::Srv);
		GDynamicRHI->CommitTextureAsView(FrameResource.DepthStencilMap.get(), FViewType::Dsv);
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
	FMatrix CamView = MainCamera.GetViewMatrix();
	FMatrix CamProj = MainCamera.GetPerspProjMatrix(1.0f, 3000.0f);

	// TODO: change the way to build Light Cam
	FVector LightPos = { 450.f, 0.f, 450.f };
	FVector LightDirNored = glm::normalize(Scene->DirectionLight.Dir);
	FVector LightTarget = LightPos + LightDirNored;
	FVector LightUpDir = { 0.f, 0.f, 1.f };

	FMatrix LightView = glm::lookAtLH(LightPos, LightTarget, LightUpDir);
	FMatrix LightProj = glm::orthoLH_ZO(-700.f, 700.f, -700.f, 700.f, 1.0f, 3000.0f);
	FMatrix LightScr(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

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

		// base pass cb
		FShadowMapCB BaseCB;
		BaseCB.World = glm::transpose(WorldMatrix);
		BaseCB.CamViewProj = glm::transpose(CamProj * CamView);
		BaseCB.ShadowTransForm = glm::transpose(LightScr * LightProj * LightView);

		FVector CamPos = Scene->SceneCamera.GetPosition();
		BaseCB.CamEye = FVector4(CamPos.x, CamPos.y, CamPos.z, 1.0f);

		BaseCB.Light.Dir = LightDirNored;
		BaseCB.Light.Color = Scene->DirectionLight.Color;
		BaseCB.Light.Intensity = Scene->DirectionLight.Intensity;
		BaseCB.IsShadowMap = FALSE;

		RHI::FCBData BasePassData;
		BasePassData.DataBuffer = reinterpret_cast<void*>(&BaseCB);
		BasePassData.BufferSize = sizeof(BaseCB);

		// shadow pass cb
		FShadowMapCB ShadowCB = BaseCB;
		ShadowCB.CamViewProj = glm::transpose(LightProj * LightView);
		ShadowCB.IsShadowMap = TRUE;

		RHI::FCBData ShadowPassData;
		ShadowPassData.DataBuffer = reinterpret_cast<void*>(&ShadowCB);
		ShadowPassData.BufferSize = sizeof(ShadowCB);

		GDynamicRHI->UpdateConstantBuffer(FrameResource.MeshActorFrameResources[MeshIndex].MeshResToRender.get(),
			&BasePassData, &ShadowPassData); // MeshActor in Scene reflect to MeshActorFrameResource by order
	}
}