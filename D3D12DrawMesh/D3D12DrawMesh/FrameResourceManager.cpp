#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include <gtc/matrix_transform.hpp>
#include "RHIResource.h"
#include "DX12Resource.h"

void FFrameResourceManager::InitFrameResource(uint32 FrameCount)
{
	FrameResources.resize(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResources[FrameIndex];

		// create postprocess mesh


		// create and commit shadow map
		FrameResource.ShadowMap = GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP, FrameResource.ShadowMapSize, FrameResource.ShadowMapSize);
		GDynamicRHI->CommitTextureAsView(FrameResource.ShadowMap.get(), FViewType::Dsv);
		GDynamicRHI->CommitTextureAsView(FrameResource.ShadowMap.get(), FViewType::Srv);

		// create and commit Ds map
		FrameResource.DepthStencilMap = GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight());
		GDynamicRHI->CommitTextureAsView(FrameResource.DepthStencilMap.get(), FViewType::Dsv);

		// create and commit sampler
		FrameResource.ClampSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP);

		// create and commit scene color
		FrameResource.SceneColor = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight());
		GDynamicRHI->CommitTextureAsView(FrameResource.SceneColor.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.SceneColor.get(), FViewType::Srv);

		// create and commit bloom down and up texture
		FrameResource.BloomSetup = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomSetup.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomSetup.get(), FViewType::Srv);

		FrameResource.Bloomdown8 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 8, GDynamicRHI->GetHeight() / 8);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown8.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown8.get(), FViewType::Srv);

		FrameResource.Bloomdown16 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 16, GDynamicRHI->GetHeight() / 16);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown16.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown16.get(), FViewType::Srv);

		FrameResource.Bloomdown32 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 32, GDynamicRHI->GetHeight() / 32);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown32.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown32.get(), FViewType::Srv);

		FrameResource.Bloomdown64 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 64, GDynamicRHI->GetHeight() / 64);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown64.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomdown64.get(), FViewType::Srv);
		
		FrameResource.Bloomup32 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 32, GDynamicRHI->GetHeight() / 32);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup32.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup32.get(), FViewType::Srv);
		
		FrameResource.Bloomup16 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 16, GDynamicRHI->GetHeight() / 16);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup16.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup16.get(), FViewType::Srv);

		FrameResource.Bloomup8 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 8, GDynamicRHI->GetHeight() / 8);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup8.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup8.get(), FViewType::Srv);
		
		FrameResource.Bloomup4 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup4.get(), FViewType::Rtv);
		GDynamicRHI->CommitTextureAsView(FrameResource.Bloomup4.get(), FViewType::Srv);

	}
}

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount)
{
	RHI::GDynamicRHI->BegineCreateResource();
	InitFrameResource(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResources[FrameIndex];

		// create mesh resource
		const uint32 MeshActorCount = static_cast<uint32>(Scene->MeshActors.size());
		FrameResource.MeshActorFrameReses.resize(MeshActorCount);
		for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
		{
			FMeshActorFrameRes& MeshActorFrameResource = FrameResource.MeshActorFrameReses[MeshIndex];
			const FMeshActor& MeshActor = Scene->MeshActors[MeshIndex];
			CreateMeshActorFrameResources(MeshActorFrameResource, MeshActor); // MeshActor in Scene reflect to MeshActorFrameResource by order
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

void FFrameResourceManager::CreateMeshActorFrameResources(FMeshActorFrameRes& MeshActorFrameResource, const FMeshActor& MeshActor)
{
	RHI::GDynamicRHI->CreateMeshForFrameResource(MeshActorFrameResource, MeshActor);
}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, const uint32& FrameIndex)
{
	FCamera& MainCamera = Scene->GetCurrentCamera();
	FMatrix CamView = MainCamera.GetViewMatrix();
	FMatrix CamProj = MainCamera.GetPerspProjMatrix(1.0f, 3000.0f);

	// TODO: change the way to build Light Cam, add a algorithm to calculate bounding box size
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

		GDynamicRHI->UpdateConstantBuffer(FrameResource.MeshActorFrameReses[MeshIndex].MeshRes.get(),
			&BasePassData, &ShadowPassData); // MeshActor in Scene reflect to MeshActorFrameResource by order
	}
}