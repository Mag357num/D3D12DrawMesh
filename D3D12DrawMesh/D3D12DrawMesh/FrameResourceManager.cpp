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
		//vector<float> TriangleVertices =
		//{ 
		//	-3.f, -1.f, 0.0f, -1.f,  1.f,
		//	 1.f,  3.f, 0.0f,  1.f, -1.f,
		//	 1.f, -1.f, 0.0f,  1.f,  1.f,
		//};
		vector<float> TriangleVertices =
		{
			 1.f, -1.f, 0.0f,  1.f,  1.f,
			 1.f,  3.f, 0.0f,  1.f, -1.f,
			-3.f, -1.f, 0.0f, -1.f,  1.f,
		};

		FMeshActor Actor = GDynamicRHI->CreateMeshActor(20, TriangleVertices, { 0, 1, 2 }, { { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f } });
		FrameResource.PostProcessTriangle = GDynamicRHI->CreateMesh(Actor);
		FrameResource.PostProcessTriangleRes = GDynamicRHI->CreateMeshRes(L"ToneMapping.hlsl", 256, FPassType::LDR_OUTPUT_RT_PT);

		// create and commit shadow map
		FrameResource.ShadowMap = GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP_TT, FrameResource.ShadowMapSize, FrameResource.ShadowMapSize);
		GDynamicRHI->CommitTextureAsView(FrameResource.ShadowMap.get(), FResViewType::DSV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.ShadowMap.get(), FResViewType::SRV_RVT);

		// create and commit Ds map
		FrameResource.DepthStencilMap = GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight());
		GDynamicRHI->CommitTextureAsView(FrameResource.DepthStencilMap.get(), FResViewType::DSV_RVT);

		// create and commit sampler
		FrameResource.ClampSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP_ST);

		// create and commit scene color
		FrameResource.SceneColorMap = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight());
		GDynamicRHI->CommitTextureAsView(FrameResource.SceneColorMap.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.SceneColorMap.get(), FResViewType::SRV_RVT);

		// create and commit bloom down and up texture
		FrameResource.BloomSetupMap = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomSetupMap.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomSetupMap.get(), FResViewType::SRV_RVT);

		FrameResource.BloomDownMap8 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 8, GDynamicRHI->GetHeight() / 8);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap8.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap8.get(), FResViewType::SRV_RVT);

		FrameResource.BloomDownMap16 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 16, GDynamicRHI->GetHeight() / 16);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap16.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap16.get(), FResViewType::SRV_RVT);

		FrameResource.BloomDownMap32 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 32, GDynamicRHI->GetHeight() / 32);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap32.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap32.get(), FResViewType::SRV_RVT);

		FrameResource.BloomDownMap64 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 64, GDynamicRHI->GetHeight() / 64);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap64.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomDownMap64.get(), FResViewType::SRV_RVT);
		
		FrameResource.BloomUpMap32 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 32, GDynamicRHI->GetHeight() / 32);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap32.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap32.get(), FResViewType::SRV_RVT);
		
		FrameResource.BloomUpMap16 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 16, GDynamicRHI->GetHeight() / 16);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap16.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap16.get(), FResViewType::SRV_RVT);

		FrameResource.BloomUpMap8 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 8, GDynamicRHI->GetHeight() / 8);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap8.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap8.get(), FResViewType::SRV_RVT);
		
		FrameResource.BloomUpMap4 = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap4.get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.BloomUpMap4.get(), FResViewType::SRV_RVT);

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

void FFrameResourceManager::CreateMeshActorFrameResources(FMeshActorFrameRes& MAFrameRes, const FMeshActor& MeshActor)
{
	RHI::GDynamicRHI->CreateMeshForFrameResource(MAFrameRes, MeshActor);
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
		const FVector& Rotate = Scene->MeshActors[MeshIndex].Transform.Rotation; // x roll y pitch z yaw

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