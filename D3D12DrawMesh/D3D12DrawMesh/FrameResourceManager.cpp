#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include "RHIResource.h"
#include "DX12Resource.h"
#include "MathExtend.h"
#include "AssetManager.h"

void FFrameResourceManager::InitFrameResource(const uint32& FrameCount)
{
	FrameResArray.resize(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex) // init res for each frame
	{
		FFrameResource& FrameRes = FrameResArray[FrameIndex];

		CreateMapsForShadow(FrameRes);
		CreateSamplers(FrameRes);
		CreateMapsForScene(FrameRes);
		CreateMapsForPostProcess(FrameRes);
		CreatePostProcessTriangle(FrameRes);
	}
}

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameNum)
{
	RHI::GDynamicRHI->BegineCreateResource();
	InitFrameResource(FrameNum);

	for (uint32 FrameIndex = 0; FrameIndex < FrameNum; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResArray[FrameIndex];

		FrameResource.GetSkeletalMesh() = CreateFrameMesh(*Scene->GetCharacter()->GetSkeletalMeshCom());

		const uint32 StaticMeshActorNum = static_cast<uint32>(Scene->GetStaticMeshActors().size());
		FrameResource.GetStaticMeshArray().resize(StaticMeshActorNum);
		for (uint32 i = 0; i < StaticMeshActorNum; ++i)
		{
			FrameResource.GetStaticMeshArray()[i] = CreateFrameMesh(*Scene->GetStaticMeshActors()[i].GetComs()[0]->As<FStaticMeshComponent>());
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

FFrameMesh FFrameResourceManager::CreateFrameMesh(FStaticMeshComponent& MeshComponent)
{
	FFrameMesh MeshComFrameRes;

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back({ "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 });
	InputLayer.Elements.push_back({ "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 });
	InputLayer.Elements.push_back({ "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 });
	InputLayer.Elements.push_back({ "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 });

	MeshComFrameRes.Mesh = GDynamicRHI->CreateMesh(MeshComponent, InputLayer);
	MeshComFrameRes.MeshRes = make_shared<FMeshRes>();

	// material
	vector<shared_ptr<FHandle>> Empty; // TODO: refactor
	MeshComFrameRes.MeshRes->ShadowMat = GDynamicRHI->CreateMaterial(MeshComponent.GetShaderFileName(), 256, Empty);
	MeshComFrameRes.MeshRes->SceneColorMat = GDynamicRHI->CreateMaterial(MeshComponent.GetShaderFileName(), 256, Empty);

	// shadow map pipeline
	FShaderInputLayer ShaderInputLayer;
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	MeshComFrameRes.MeshRes->ShadowPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_UNKNOWN, 0, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->ShadowMat.get());

	// shadow map pipeline
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	MeshComFrameRes.MeshRes->SceneColorPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R16G16B16A16_FLOAT, 1, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->SceneColorMat.get());

	return MeshComFrameRes;
}

FFrameMesh FFrameResourceManager::CreateFrameMesh(FSkeletalMeshComponent& MeshComponent)
{
	FFrameMesh MeshComFrameRes;

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back({ "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 });
	InputLayer.Elements.push_back({ "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 });
	InputLayer.Elements.push_back({ "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 });
	InputLayer.Elements.push_back({ "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 });

	MeshComFrameRes.Mesh = GDynamicRHI->CreateMesh(MeshComponent, InputLayer);
	MeshComFrameRes.MeshRes = make_shared<FMeshRes>();

	// material
	vector<shared_ptr<FHandle>> Empty; // TODO: refactor
	MeshComFrameRes.MeshRes->ShadowMat = GDynamicRHI->CreateMaterial(MeshComponent.GetShaderFileName(), 256, Empty);
	MeshComFrameRes.MeshRes->SceneColorMat = GDynamicRHI->CreateMaterial(MeshComponent.GetShaderFileName(), 256, Empty);

	// shadow map pipeline
	FShaderInputLayer ShaderInputLayer;
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	MeshComFrameRes.MeshRes->ShadowPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_UNKNOWN, 0, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->ShadowMat.get());

	// shadow map pipeline
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	MeshComFrameRes.MeshRes->SceneColorPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R16G16B16A16_FLOAT, 1, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->SceneColorMat.get());

	return MeshComFrameRes;
}

void FFrameResourceManager::CreateMapsForShadow(FFrameResource& FrameRes)
{
	// create and commit null texture
	FrameRes.SetNullTexture(GDynamicRHI->CreateTexture(FTextureType::ORDINARY_SHADER_RESOURCE_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight()));
	GDynamicRHI->CommitTextureAsView(FrameRes.GetNullTexture().get(), FResViewType::SRV_RVT);

	// create and commit shadow map
	FrameRes.SetShadowMap(GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP_TT, FrameRes.GetShadowMapSize(), FrameRes.GetShadowMapSize()));
	GDynamicRHI->CommitTextureAsView(FrameRes.GetShadowMap().get(), FResViewType::DSV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.GetShadowMap().get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreateSamplers(FFrameResource& FrameRes)
{
	// create and commit sampler
	FrameRes.SetClampSampler(GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP_ST));
	FrameRes.SetWarpSampler(GDynamicRHI->CreateAndCommitSampler(FSamplerType::WARP_ST));
}

void FFrameResourceManager::CreateMapsForScene(FFrameResource& FrameRes)
{
	// create and commit Ds map
	FrameRes.SetDsMap(GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight()));
	GDynamicRHI->CommitTextureAsView(FrameRes.GetDsMap().get(), FResViewType::DSV_RVT);

	// create and commit scene color
	FrameRes.SetSceneColorMap(GDynamicRHI->CreateTexture(FTextureType::SCENE_COLOR_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight()));
	GDynamicRHI->CommitTextureAsView(FrameRes.GetSceneColorMap().get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.GetSceneColorMap().get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreateMapsForPostProcess(FFrameResource& FrameRes)
{
	// create and commit bloom down and up texture
	FrameRes.SetBloomSetupMap(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4));
	GDynamicRHI->CommitTextureAsView(FrameRes.GetBloomSetupMap().get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.GetBloomSetupMap().get(), FResViewType::SRV_RVT);

	for (uint32 i = 0; i < 4; i++)
	{
		uint32 ShrinkTimes = static_cast<uint32>(pow(2, 3 + i));
		FrameRes.GetBloomDownMapArray().push_back(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / ShrinkTimes, GDynamicRHI->GetHeight() / ShrinkTimes));
		GDynamicRHI->CommitTextureAsView(FrameRes.GetBloomDownMapArray()[i].get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameRes.GetBloomDownMapArray()[i].get(), FResViewType::SRV_RVT);
	}

	for (int i = 4; i > 1; i--)
	{
		uint32 ShrinkTimes = static_cast<uint32>(pow(2, 1 + i));
		FrameRes.GetBloomUpMapArray().push_back(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / ShrinkTimes, GDynamicRHI->GetHeight() / ShrinkTimes));
		GDynamicRHI->CommitTextureAsView(FrameRes.GetBloomUpMapArray()[4 - i].get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameRes.GetBloomUpMapArray()[4 - i].get(), FResViewType::SRV_RVT);
	}

	FrameRes.SetSunMergeMap(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4));
	GDynamicRHI->CommitTextureAsView(FrameRes.GetSunMergeMap().get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.GetSunMergeMap().get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreatePostProcessTriangle(FFrameResource& FrameRes)
{
	// create postprocess mesh and mesh resource
	vector<FStaticVertex> TriangleVertice;
	TriangleVertice.push_back(FStaticVertex(FVector(1.f, -1.f, 0.0f), FVector(1, 1, 1), FVector2(1.f, 1.f), FVector4(1, 1, 1, 1)));
	TriangleVertice.push_back(FStaticVertex(FVector(1.f, 3.f, 0.0f), FVector(1, 1, 1), FVector2(1.f, -1.f), FVector4(1, 1, 1, 1)));
	TriangleVertice.push_back(FStaticVertex(FVector(-3.f, -1.f, 0.0f), FVector(1, 1, 1), FVector2(-1.f, 1.f), FVector4(1, 1, 1, 1)));

	vector<uint32> Indice = { 0, 1, 2 };

	FStaticMeshComponent Component = FAssetManager::Get()->CreateStaticMeshComponent(TriangleVertice, Indice,
		{ { 1.f, 1.f, 1.f }, EulerToQuat({0.f, 0.f, 0.f}), { 0.f, 0.f, 0.f } }); // didnt use triangle's transform

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back( { "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 } );
	InputLayer.Elements.push_back( { "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 } );
	InputLayer.Elements.push_back( { "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 } );
	InputLayer.Elements.push_back( { "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 } );

	FrameRes.SetPostProcessTriangle(GDynamicRHI->CreateMesh(Component, InputLayer));
	FrameRes.SetPostProcessTriangleRes(make_shared<FMeshRes>());

	CreatePostProcessMaterials(FrameRes);
	CreatePostProcessPipelines(FrameRes);
}

void FFrameResourceManager::CreatePostProcessMaterials(FFrameResource& FrameRes)
{
	FMeshRes* TriRes = FrameRes.GetPostProcessTriangleRes().get();
	FMesh* Tri = FrameRes.GetPostProcessTriangle().get();

	// create postprocess material
	// bloom setup
	vector<shared_ptr<FHandle>> TexHandles;
	TexHandles.push_back(FrameRes.GetSceneColorMap()->SrvHandle);
	TriRes->BloomSetupMat = GDynamicRHI->CreateMaterial(L"BloomSetup.hlsl", 256, TexHandles);

	// bloom down
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomSetupMap()->SrvHandle);
	TriRes->BloomDownMat[0] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[0]->SrvHandle);
	TriRes->BloomDownMat[1] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[1]->SrvHandle);
	TriRes->BloomDownMat[2] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[2]->SrvHandle);
	TriRes->BloomDownMat[3] = GDynamicRHI->CreateMaterial(L"BloomDownMat.hlsl", 256, TexHandles);

	// bloom up
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[2]->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[3]->SrvHandle);
	TriRes->BloomUpMat[0] = GDynamicRHI->CreateMaterial(L"BloomUpMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[1]->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomUpMapArray()[0]->SrvHandle);
	TriRes->BloomUpMat[1] = GDynamicRHI->CreateMaterial(L"BloomUpMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[0]->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomUpMapArray()[1]->SrvHandle);
	TriRes->BloomUpMat[2] = GDynamicRHI->CreateMaterial(L"BloomUpMat.hlsl", 256, TexHandles);

	// sun merge
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomSetupMap()->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomUpMapArray()[2]->SrvHandle);
	TriRes->SunMergeMat = GDynamicRHI->CreateMaterial(L"SunMerge.hlsl", 256, TexHandles);

	// tonemapping
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetSceneColorMap()->SrvHandle);
	TexHandles.push_back(FrameRes.GetSunMergeMap()->SrvHandle);
	TriRes->ToneMappingMat = GDynamicRHI->CreateMaterial(L"ToneMapping.hlsl", 256, TexHandles);
}

void FFrameResourceManager::CreatePostProcessPipelines(FFrameResource& FrameRes)
{
	FMeshRes* TriRes = FrameRes.GetPostProcessTriangleRes().get();
	FMesh* Tri = FrameRes.GetPostProcessTriangle().get();

	// pipeline
	// bloom setup
	FShaderInputLayer ShaderInputLayer;
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	TriRes->BloomSetupPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, TriRes->BloomSetupMat.get());

	// bloom down
	TriRes->BloomDownPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, TriRes->BloomDownMat[0].get());

	// bloom up
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	TriRes->BloomUpPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, TriRes->BloomUpMat[0].get());

	// sun merge
	TriRes->SunMergePipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, TriRes->SunMergeMat.get());

	// tone mapping
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	TriRes->ToneMappingPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R8G8B8A8_UNORM, 1, Tri->InputLayer, ShaderInputLayer, TriRes->ToneMappingMat.get());

}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, const uint32& FrameIndex)
{
	FFrameResource& FrameRes = FrameResArray[FrameIndex];

	ACamera& MainCamera = Scene->GetCurrentCamera();
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

	const uint32 ActorNum = static_cast<uint32>(Scene->GetStaticMeshActors().size());
	for (uint32 MeshIndex = 0; MeshIndex < ActorNum; ++MeshIndex)
	{
		const FRotator& Rotate = QuatToEuler(Scene->GetStaticMeshActors()[MeshIndex].GetComs()[0]->GetTransform().Quat); // x roll y pitch z yaw

		FMatrix RotateMatrix;
		RotateMatrix = glm::rotate(glm::identity<FMatrix>(), glm::radians(-Rotate.Roll), FVector(1, 0, 0)); // roll
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(-Rotate.Pitch), FVector(0, 1, 0)); // pitch
		RotateMatrix = glm::rotate(RotateMatrix, glm::radians(Rotate.Yaw), FVector(0, 0, 1)); // yaw

		FMatrix ScaleMatrix = glm::scale(glm::identity<FMatrix>(), Scene->GetStaticMeshActors()[MeshIndex].GetComs()[0]->GetTransform().Scale);
		FMatrix TranslateMatrix = glm::translate(glm::identity<FMatrix>(), Scene->GetStaticMeshActors()[MeshIndex].GetComs()[0]->GetTransform().Translation);

		FMatrix WorldMatrix = TranslateMatrix * RotateMatrix * ScaleMatrix; // use column matrix, multiple is right to left

		// base pass cb
		FSceneColorCB SceneColorCB;
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
		FSceneColorCB ShadowCbStruct = SceneColorCB;
		ShadowCbStruct.CamViewProj = glm::transpose(LightProj * LightView);
		ShadowCbStruct.IsShadowMap = TRUE;
		RHI::FCBData ShadowPassData;
		ShadowPassData.DataBuffer = reinterpret_cast<void*>(&ShadowCbStruct);
		ShadowPassData.BufferSize = sizeof(ShadowCbStruct);

		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetStaticMeshArray()[MeshIndex].MeshRes->ShadowMat.get(), &ShadowPassData);
		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetStaticMeshArray()[MeshIndex].MeshRes->SceneColorMat.get(), &SceneColorPassData);
	}

	// skeletalmesh
	{
		FMatrix ScaleMatrix = glm::scale(glm::identity<FMatrix>(), Scene->GetCharacter()->GetSkeletalMeshCom()->GetTransform().Scale);
		FMatrix Quat = glm::toMat4(Scene->GetCharacter()->GetSkeletalMeshCom()->GetTransform().Quat);
		FMatrix TranslateMatrix = glm::translate(glm::identity<FMatrix>(), Scene->GetCharacter()->GetSkeletalMeshCom()->GetTransform().Translation);
		FMatrix WorldMatrix = TranslateMatrix * Quat * ScaleMatrix; // use column matrix, multiple is right to left

		FSceneColorCB SceneColorCB;
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
		FSceneColorCB ShadowCbStruct = SceneColorCB;
		ShadowCbStruct.CamViewProj = glm::transpose(LightProj * LightView);
		ShadowCbStruct.IsShadowMap = TRUE;
		RHI::FCBData ShadowPassData;
		ShadowPassData.DataBuffer = reinterpret_cast<void*>(&ShadowCbStruct);
		ShadowPassData.BufferSize = sizeof(ShadowCbStruct);

		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetSkeletalMesh().MeshRes->ShadowMat.get(), &ShadowPassData);
		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetSkeletalMesh().MeshRes->SceneColorMat.get(), &SceneColorPassData);
	}



	// update postprocess
	FVector2 WidthAndHeight = FVector2(static_cast<float>(GDynamicRHI->GetWidth()), static_cast<float>(GDynamicRHI->GetHeight()));
	
	// bloom setup
	FBloomSetupCB BloomSetupStruct;
	BloomSetupStruct.BufferSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / 4.f);
	BloomSetupStruct.BloomThreshold = 1.0f;
	RHI::FCBData BloomSetupPassData;
	BloomSetupPassData.DataBuffer = reinterpret_cast<void*>(&BloomSetupStruct);
	BloomSetupPassData.BufferSize = sizeof(BloomSetupStruct);
	GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->BloomSetupMat.get(), &BloomSetupPassData);

	// bloom down
	for (uint32 i = 0; i < 4; i++)
	{
		FBloomDownCB BloomDwonStruct;
		BloomDwonStruct.BufferSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight  / (4.f * static_cast<float>(pow(2, i + 1))));
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
		BloomUpStruct.BufferASizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / (4.f * static_cast<float>(pow(2, i + 1))));
		BloomUpStruct.BufferBSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / (4.f * static_cast<float>(pow(2, i + 2))));
		BloomUpStruct.BloomTintA = BloomTintAs[2 - i] * (1.0f / 8.0f);
		BloomUpStruct.BloomTintB = BloomTintBs[2 - i] * (1.0f / 8.0f);
		BloomUpStruct.BloomUpScales.x = 0.66f * 2.0f;
		BloomUpStruct.BloomUpScales.y = 0.66f * 2.0f;
		RHI::FCBData BloomUpPassData;
		BloomUpPassData.DataBuffer = reinterpret_cast<void*>(&BloomUpStruct);
		BloomUpPassData.BufferSize = sizeof(BloomUpStruct);
		GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->BloomUpMat[2-i].get(), &BloomUpPassData);
	}

	// sun merge
	FSunMergeCB SunMergeStruct;
	SunMergeStruct.BloomUpSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / 8.f);
	SunMergeStruct.BloomColor = FVector(BloomTint1) * BloomIntensity * 0.5f;
	RHI::FCBData SunMergePassData;
	SunMergePassData.DataBuffer = reinterpret_cast<void*>(&SunMergeStruct);
	SunMergePassData.BufferSize = sizeof(SunMergeStruct);
	GDynamicRHI->UpdateConstantBuffer(FrameRes.GetPostProcessTriangleRes()->SunMergeMat.get(), &SunMergePassData);
}