#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include <gtc/matrix_transform.hpp>
#include "RHIResource.h"
#include "DX12Resource.h"

void FFrameResourceManager::InitFrameResource(const uint32& FrameCount)
{
	GetFrameRes().resize(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = GetFrameRes()[FrameIndex];

		// create and commit shadow map
		FrameResource.SetShadowMap(GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP_TT, FrameResource.GetShadowMapSize(), FrameResource.GetShadowMapSize()));
		GDynamicRHI->CommitTextureAsView(FrameResource.GetShadowMap().get(), FResViewType::DSV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.GetShadowMap().get(), FResViewType::SRV_RVT);

		// create and commit Ds map
		FrameResource.SetDsMap(GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight()));
		GDynamicRHI->CommitTextureAsView(FrameResource.GetDsMap().get(), FResViewType::DSV_RVT);

		// create and commit sampler
		FrameResource.SetClampSampler(GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP_ST));
		FrameResource.SetWarpSampler(GDynamicRHI->CreateAndCommitSampler(FSamplerType::WARP_ST));

		// create and commit scene color
		FrameResource.SetSceneColorMap(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight()));
		GDynamicRHI->CommitTextureAsView(FrameResource.GetSceneColorMap().get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.GetSceneColorMap().get(), FResViewType::SRV_RVT);

		// create and commit bloom down and up texture
		FrameResource.SetBloomSetupMap(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4));
		GDynamicRHI->CommitTextureAsView(FrameResource.GetBloomSetupMap().get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.GetBloomSetupMap().get(), FResViewType::SRV_RVT);

		for (uint32 i = 0; i < 4; i++)
		{
			uint32 ZoomIn = static_cast<uint32>(pow( 2, 3 + i ));
			FrameResource.GetBloomDownMaps().push_back(GDynamicRHI->CreateTexture( FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / ZoomIn, GDynamicRHI->GetHeight() / ZoomIn ));
			GDynamicRHI->CommitTextureAsView( FrameResource.GetBloomDownMaps()[i].get(), FResViewType::RTV_RVT );
			GDynamicRHI->CommitTextureAsView( FrameResource.GetBloomDownMaps()[i].get(), FResViewType::SRV_RVT );
		}

		for (int i = 4; i > 1; i--)
		{
			uint32 ZoomIn = static_cast<uint32>(pow( 2, 1 + i ));
			FrameResource.GetBloomUpMaps().push_back( GDynamicRHI->CreateTexture( FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / ZoomIn, GDynamicRHI->GetHeight() / ZoomIn ) );
			GDynamicRHI->CommitTextureAsView( FrameResource.GetBloomUpMaps()[4-i].get(), FResViewType::RTV_RVT );
			GDynamicRHI->CommitTextureAsView( FrameResource.GetBloomUpMaps()[4-i].get(), FResViewType::SRV_RVT );
		}
		
		FrameResource.SetSunMergeMap(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4));
		GDynamicRHI->CommitTextureAsView(FrameResource.GetSunMergeMap().get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameResource.GetSunMergeMap().get(), FResViewType::SRV_RVT);

		// create postprocess mesh and mesh resource
		vector<float> TriangleVertices =
		{
			 1.f, -1.f, 0.0f,  1.f,  1.f,
			 1.f,  3.f, 0.0f,  1.f, -1.f,
			-3.f, -1.f, 0.0f, -1.f,  1.f,
		};
		FMeshActor Actor = GDynamicRHI->CreateMeshActor(20, TriangleVertices, { 0, 1, 2 }, { { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f } });
		FrameResource.SetPostProcessTriangle( GDynamicRHI->CreateMesh( Actor ) );
		FrameResource.SetPostProcessTriangleRes( GDynamicRHI->CreateMeshRes() );

		// create postprocess material
		// bloom setup
		vector<shared_ptr<FHandle>> TexHandles;
		TexHandles.push_back(FrameResource.GetSceneColorMap()->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomSetupMat = GDynamicRHI->CreateMaterial( L"BloomSetup.hlsl", 256, TexHandles, FPassType::BLOOM_SETUP_PT );
		TexHandles.clear();

		// bloom down
		TexHandles.push_back(FrameResource.GetBloomSetupMap()->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomDownMat[0] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles, FPassType::BLOOM_DOWN_PT);
		TexHandles.clear();

		TexHandles.push_back(FrameResource.GetBloomDownMaps()[0]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomDownMat[1] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles, FPassType::BLOOM_DOWN_PT);
		TexHandles.clear();

		TexHandles.push_back(FrameResource.GetBloomDownMaps()[1]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomDownMat[2] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles, FPassType::BLOOM_DOWN_PT);
		TexHandles.clear();

		TexHandles.push_back(FrameResource.GetBloomDownMaps()[2]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomDownMat[3] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles, FPassType::BLOOM_DOWN_PT);
		TexHandles.clear();

		// bloom up
		TexHandles.push_back(FrameResource.GetBloomDownMaps()[2]->SrvHandle);
		TexHandles.push_back(FrameResource.GetBloomDownMaps()[3]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomUpMat[0] = GDynamicRHI->CreateMaterial(L"BloomUpMat.hlsl", 256, TexHandles, FPassType::BLOOM_UP_PT);
		TexHandles.clear();

		TexHandles.push_back(FrameResource.GetBloomDownMaps()[1]->SrvHandle);
		TexHandles.push_back(FrameResource.GetBloomUpMaps()[0]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomUpMat[1] = GDynamicRHI->CreateMaterial(L"BloomUpMat.hlsl", 256, TexHandles, FPassType::BLOOM_UP_PT);
		TexHandles.clear();

		TexHandles.push_back(FrameResource.GetBloomDownMaps()[0]->SrvHandle);
		TexHandles.push_back(FrameResource.GetBloomUpMaps()[1]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->BloomUpMat[2] = GDynamicRHI->CreateMaterial(L"BloomUpMat.hlsl", 256, TexHandles, FPassType::BLOOM_UP_PT);
		TexHandles.clear();

		// sun merge
		TexHandles.push_back(FrameResource.GetBloomSetupMap()->SrvHandle);
		TexHandles.push_back(FrameResource.GetBloomUpMaps()[2]->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->SunMergeMat = GDynamicRHI->CreateMaterial(L"SunMerge.hlsl", 256, TexHandles, FPassType::SUN_MERGE_PT);
		TexHandles.clear();

		// tonemapping
		TexHandles.push_back(FrameResource.GetSceneColorMap()->SrvHandle);
		TexHandles.push_back(FrameResource.GetSunMergeMap()->SrvHandle);
		FrameResource.GetPostProcessTriangleRes()->ToneMappingMat = GDynamicRHI->CreateMaterial(L"ToneMapping.hlsl", 256, TexHandles, FPassType::TONEMAPPING_PT);
		TexHandles.clear();
	}
}

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameCount)
{
	RHI::GDynamicRHI->BegineCreateResource();
	InitFrameResource(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FFrameResource& FrameResource = GetFrameRes()[FrameIndex];

		// create mesh resource
		const uint32 MeshActorCount = static_cast<uint32>(Scene->GetMeshActors().size());
		FrameResource.GetFrameMeshes().resize(MeshActorCount);
		for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
		{
			FFrameMesh& MeshActorFrameResource = FrameResource.GetFrameMeshes()[MeshIndex];
			const FMeshActor& MeshActor = Scene->GetMeshActors()[MeshIndex];
			CreateMeshActorFrameResources(MeshActorFrameResource, MeshActor); // MeshActor in Scene reflect to MeshActorFrameResource by order
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

void FFrameResourceManager::CreateMeshActorFrameResources(FFrameMesh& MAFrameRes, const FMeshActor& MeshActor)
{
	RHI::GDynamicRHI->CreateFrameMesh(MAFrameRes, MeshActor);
}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, const uint32& FrameIndex)
{
	FFrameResource& FrameRes = GetFrameRes()[FrameIndex];

	FCamera& MainCamera = Scene->GetCurrentCamera();
	FMatrix CamView = MainCamera.GetViewMatrix();
	FMatrix CamProj = MainCamera.GetPerspProjMatrix(1.0f, 3000.0f);

	// TODO: change the way to build Light Cam, add a algorithm to calculate bounding box size
	FVector LightPos = { 450.f, 0.f, 450.f };
	FVector LightDirNored = glm::normalize(Scene->GetDirectionLight().Dir);
	FVector LightTarget = LightPos + LightDirNored;
	FVector LightUpDir = { 0.f, 0.f, 1.f };

	FMatrix LightView = glm::lookAtLH(LightPos, LightTarget, LightUpDir);
	FMatrix LightProj = glm::orthoLH_ZO(-700.f, 700.f, -700.f, 700.f, 1.0f, 3000.0f);
	FMatrix LightScr(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	const uint32 MeshActorCount = static_cast<uint32>(Scene->GetMeshActors().size());
	for (uint32 MeshIndex = 0; MeshIndex < MeshActorCount; ++MeshIndex)
	{
		const FMatrix Identity = glm::identity<FMatrix>();
		const FVector& Rotate = Scene->GetMeshActors()[MeshIndex].Transform.Rotation; // x roll y pitch z yaw

		FMatrix RotateMatrix = Identity;
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.x), FVector(1, 0, 0)); // roll
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.y), FVector(0, 1, 0)); // pitch
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(Rotate.z), FVector(0, 0, 1)); // yaw

		FMatrix ScaleMatrix = glm::scale(Identity, Scene->GetMeshActors()[MeshIndex].Transform.Scale);
		FMatrix TranslateMatrix = glm::translate(Identity, Scene->GetMeshActors()[MeshIndex].Transform.Translation);

		FMatrix WorldMatrix = Identity * TranslateMatrix * RotateMatrix * ScaleMatrix; // use column matrix, multiple is right to left

		// base pass cb
		FShadowMapCB SceneColorCB;
		SceneColorCB.World = glm::transpose(WorldMatrix);
		SceneColorCB.CamViewProj = glm::transpose(CamProj * CamView);
		SceneColorCB.ShadowTransForm = glm::transpose(LightScr * LightProj * LightView);
		SceneColorCB.CamEye = FVector4(Scene->GetCurrentCamera().GetPosition().x, Scene->GetCurrentCamera().GetPosition().y, Scene->GetCurrentCamera().GetPosition().z, 1.0f);
		SceneColorCB.Light.Dir = LightDirNored;
		SceneColorCB.Light.Color = Scene->GetDirectionLight().Color;
		SceneColorCB.Light.Intensity = Scene->GetDirectionLight().Intensity;
		SceneColorCB.IsShadowMap = FALSE;
		RHI::FCBData SceneColorPassData;
		SceneColorPassData.DataBuffer = reinterpret_cast<void*>(&SceneColorCB);
		SceneColorPassData.BufferSize = sizeof(SceneColorCB);

		// shadow pass cb
		FShadowMapCB ShadowCbStruct = SceneColorCB;
		ShadowCbStruct.CamViewProj = glm::transpose(LightProj * LightView);
		ShadowCbStruct.IsShadowMap = TRUE;
		RHI::FCBData ShadowPassData;
		ShadowPassData.DataBuffer = reinterpret_cast<void*>(&ShadowCbStruct);
		ShadowPassData.BufferSize = sizeof(ShadowCbStruct);

		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetFrameMeshes()[MeshIndex].MeshRes->ShadowMat.get(), &ShadowPassData);
		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetFrameMeshes()[MeshIndex].MeshRes->SceneColorMat.get(), &SceneColorPassData);
	}

	FVector4 BufferSizeAndInvSize = FVector4(static_cast<float>(GDynamicRHI->GetWidth()), static_cast<float>(GDynamicRHI->GetHeight()),
		1.f / static_cast<float>(GDynamicRHI->GetWidth()), 1.f / static_cast<float>(GDynamicRHI->GetWidth()));

	// update postprocess
	// bloom setup
	FBloomSetupCB BloomSetupStruct;
	BloomSetupStruct.BufferSizeAndInvSize = BufferSizeAndInvSize / 4.f;
	BloomSetupStruct.BloomThreshold = 1.0f;
	RHI::FCBData BloomSetupPassData;
	BloomSetupPassData.DataBuffer = reinterpret_cast<void*>(&BloomSetupStruct);
	BloomSetupPassData.BufferSize = sizeof(BloomSetupStruct);
	GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->BloomSetupMat.get(), &BloomSetupPassData);

	// bloom down
	for (uint32 i = 0; i < 4; i++)
	{
		FBloomDownCB BloomDwonStruct;
		BloomDwonStruct.BufferSizeAndInvSize = BufferSizeAndInvSize / (4.f * static_cast<float>(pow(2, i + 1)));
		BloomDwonStruct.BloomDownScale = 0.66f * 4.0f;
		RHI::FCBData BloomDwonPassData;
		BloomDwonPassData.DataBuffer = reinterpret_cast<void*>(&BloomDwonStruct);
		BloomDwonPassData.BufferSize = sizeof(BloomDwonStruct);
		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->BloomDownMat[i].get(), &BloomDwonPassData);
	}

	// bloom up
	static const FVector4 BloomTint1 = FVector4(0.3465f);
	static const FVector4 BloomTint2 = FVector4(0.138f);
	static const FVector4 BloomTint3 = FVector4(0.1176f);
	static const FVector4 BloomTint4 = FVector4(0.066f);
	static const FVector4 BloomTint5 = FVector4(0.066f);
	static const float BloomIntensity = 1.0f;
	static const FVector4 BloomTintAs[3] = { BloomTint4, BloomTint3 * BloomIntensity, BloomTint2 * BloomIntensity };
	static const FVector4 BloomTintBs[3] = { BloomTint5, FVector4(1.0f, 1.0f, 1.0f, 0.0f), FVector4(1.0f, 1.0f, 1.0f, 0.0f) };
	for (int i = 2; i >= 0; i--)
	{
		FBloomUpCB BloomUpStruct;
		BloomUpStruct.BufferASizeAndInvSize = BufferSizeAndInvSize / (4.f * static_cast<float>(pow(2, i + 1)));
		BloomUpStruct.BufferBSizeAndInvSize = BufferSizeAndInvSize / (4.f * static_cast<float>(pow(2, i + 2)));
		BloomUpStruct.BloomTintA = BloomTintAs[2 - i] * (1.0f / 8.0f);
		BloomUpStruct.BloomTintB = BloomTintBs[2 - i] * (1.0f / 8.0f);
		BloomUpStruct.BloomUpScales.x = 0.66f * 2.0f;
		BloomUpStruct.BloomUpScales.y = 0.66f * 2.0f;
		RHI::FCBData BloomUpPassData;
		BloomUpPassData.DataBuffer = reinterpret_cast<void*>(&BloomUpStruct);
		BloomUpPassData.BufferSize = sizeof(BloomUpStruct);
		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->BloomUpMat[i].get(), &BloomUpPassData);
	}

	// sun merge
	FSunMergeCB SunMergeStruct;
	SunMergeStruct.BloomUpSizeAndInvSize = BufferSizeAndInvSize / 8.f;
	SunMergeStruct.BloomColor = FVector(BloomTint1) * BloomIntensity * 0.5f;
	RHI::FCBData SunMergePassData;
	SunMergePassData.DataBuffer = reinterpret_cast<void*>(&SunMergeStruct);
	SunMergePassData.BufferSize = sizeof(SunMergeStruct);
	GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->SunMergeMat.get(), &SunMergePassData);


}