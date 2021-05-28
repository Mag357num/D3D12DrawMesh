#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include "RHIResource.h"
#include "DX12Resource.h"
#include "MathExtend.h"
#include "AssetManager.h"

void FFrameResourceManager::InitFrameResource(FScene* Scene, const uint32& FrameCount)
{
	FrameResArray.resize(FrameCount);
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex) // init res for each frame
	{
		FFrameResource& FrameRes = FrameResArray[FrameIndex];

		InitCameraConstantBuffer(Scene, FrameRes);
		InitLightConstantBuffer(Scene, FrameRes);
		InitPaletteConstantBuffer(Scene, FrameRes);
		CreateMapsForShadow(FrameRes);
		CreateSamplers(FrameRes);
		CreateMapsForScene(FrameRes);
		CreateMapsForPostProcess(FrameRes);
		CreatePostProcessTriangle(FrameRes);
		CreatePostProcessMaterials(FrameRes);
		InitPostProcessConstantBuffer(FrameRes);
		CreatePostProcessPipelines(FrameRes);
	}
}

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<FScene> Scene, const uint32& FrameNum)
{
	RHI::GDynamicRHI->BegineCreateResource();
	InitFrameResource(Scene.get(), FrameNum);

	for (uint32 FrameIndex = 0; FrameIndex < FrameNum; ++FrameIndex)
	{
		FFrameResource& FrameResource = FrameResArray[FrameIndex];

		FrameResource.GetSkeletalMesh() = CreateFrameMesh(*Scene->GetCharacter()->GetSkeletalMeshCom());

		const uint32 StaticMeshActorNum = static_cast<uint32>(Scene->GetStaticMeshActors().size());
		FrameResource.GetStaticMeshArray().resize(StaticMeshActorNum);
		for (uint32 i = 0; i < StaticMeshActorNum; ++i)
		{
			FrameResource.GetStaticMeshArray()[i] = CreateFrameMesh(*Scene->GetStaticMeshActors()[i].GetComs()[0]->As<TStaticMeshComponent>());
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

FFrameMesh FFrameResourceManager::CreateFrameMesh(TStaticMeshComponent& MeshComponent)
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
	vector<shared_ptr<FHandle>> Empty;
	MeshComFrameRes.MeshRes->ShadowMat = GDynamicRHI->CreateMaterial(L"Resource\\ShadowMapping_StaticMesh.hlsl", 256, Empty);
	MeshComFrameRes.MeshRes->SceneColorMat = GDynamicRHI->CreateMaterial(L"Resource\\SceneColor_StaticMesh.hlsl", 256, Empty);

	// write world matrix to constant buffer of SceneColorMat, ShadowMat dont need cb data
	FMatrix WorldMatrix = transpose(translate(MeshComponent.GetTransform().Translation) * toMat4(MeshComponent.GetTransform().Quat) * scale(MeshComponent.GetTransform().Scale));
	GDynamicRHI->WriteConstantBuffer(MeshComFrameRes.MeshRes->SceneColorMat->CB.get(), reinterpret_cast<void*>(&WorldMatrix), sizeof(FMatrix));

	// shadow map pipeline
	FShaderInputLayer ShaderInputLayer;
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL });
	MeshComFrameRes.MeshRes->ShadowPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_UNKNOWN, 0, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->ShadowMat.get());

	// scene color pipeline
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	MeshComFrameRes.MeshRes->SceneColorPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R16G16B16A16_FLOAT, 1, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->SceneColorMat.get());

	return MeshComFrameRes;
}

FFrameMesh FFrameResourceManager::CreateFrameMesh(TSkeletalMeshComponent& MeshComponent)
{
	FFrameMesh MeshComFrameRes;

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back({ "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 });
	InputLayer.Elements.push_back({ "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 });
	InputLayer.Elements.push_back({ "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 });
	InputLayer.Elements.push_back({ "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 });
	InputLayer.Elements.push_back({ "WEIGHTS", 0, FFormat::FORMAT_R16G16B16A16_UINT, 0, 48, 0, 0 }); // array<uint16, 4>
	InputLayer.Elements.push_back({ "BONEINDICES", 0, FFormat::FORMAT_R16G16B16A16_UINT, 0, 56, 0, 0 }); // array<uint16, 4>

	MeshComFrameRes.Mesh = GDynamicRHI->CreateMesh(MeshComponent, InputLayer);
	MeshComFrameRes.MeshRes = make_shared<FMeshRes>();

	// material
	vector<shared_ptr<FHandle>> Empty;
	MeshComFrameRes.MeshRes->ShadowMat = GDynamicRHI->CreateMaterial(L"Resource\\ShadowMapping_SkeletalMesh.hlsl", 256, Empty);
	MeshComFrameRes.MeshRes->SceneColorMat = GDynamicRHI->CreateMaterial(L"Resource\\SceneColor_SkeletalMesh.hlsl", 256, Empty);

	// write world matrix to constant buffer
	FMatrix WorldMatrix = transpose(translate(MeshComponent.GetTransform().Translation) * toMat4(MeshComponent.GetTransform().Quat) * scale(MeshComponent.GetTransform().Scale));
	GDynamicRHI->WriteConstantBuffer(MeshComFrameRes.MeshRes->SceneColorMat->CB.get(), reinterpret_cast<void*>(&WorldMatrix), sizeof(FMatrix));

	// shadow map pipeline
	FShaderInputLayer ShaderInputLayer;
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL });
	MeshComFrameRes.MeshRes->ShadowPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_UNKNOWN, 0, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->ShadowMat.get());

	// shadow map pipeline
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	MeshComFrameRes.MeshRes->SceneColorPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R16G16B16A16_FLOAT, 1, MeshComFrameRes.Mesh->InputLayer, ShaderInputLayer, MeshComFrameRes.MeshRes->SceneColorMat.get());

	return MeshComFrameRes;
}

void FFrameResourceManager::InitCameraConstantBuffer(FScene* Scene, FFrameResource& FrameRes)
{
	FrameRes.SetCameraCB(GDynamicRHI->CreateConstantBuffer(256));

	FMatrix CamView = Scene->GetCurrentCamera().GetViewMatrix();
	FMatrix CamProj = Scene->GetCurrentCamera().GetPerspProjMatrix(1.0f, 3000.0f);
	FMatrix CamVP = glm::transpose(CamProj * CamView);
	FVector4 Eye(Scene->GetCurrentCamera().GetPosition().x, Scene->GetCurrentCamera().GetPosition().y, Scene->GetCurrentCamera().GetPosition().z, 1.f);

	struct CameraCB
	{
		FMatrix CamVP;
		FVector4 Eye;
	} CBInstance = { CamVP, Eye };

	GDynamicRHI->WriteConstantBuffer(FrameRes.GetCameraCB().get(), reinterpret_cast<void*>(&CBInstance), sizeof(CameraCB));
}

void FFrameResourceManager::InitLightConstantBuffer(FScene* Scene, FFrameResource& FrameRes)
{
	FrameRes.SetLightCB(GDynamicRHI->CreateConstantBuffer(256));

	FVector LightPos = { 450.f, 0.f, 450.f }; // TODO: need to calculate bounding box of scene and determine this LightPos
	FVector LightDirNored = glm::normalize(Scene->GetDirectionLight().Dir);
	FVector LightTarget = LightPos + LightDirNored;
	FVector LightUpDir = { 0.f, 0.f, 1.f };

	FMatrix LightView = glm::lookAtLH(LightPos, LightTarget, LightUpDir);
	FMatrix LightProj = glm::orthoLH_ZO(-700.f, 700.f, -700.f, 700.f, 1.0f, 3000.0f); // TODO: need to calculate bounding box of scene and determine this LightProj
	FMatrix LightScr(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f); // projection space to screen space transform

	struct LightCB
	{
		FMatrix VPMatrix;
		FMatrix ScreenMatrix;
		FDirectionLight Light;
	} CBInstance;

	CBInstance.VPMatrix = glm::transpose(LightProj * LightView);
	CBInstance.ScreenMatrix = glm::transpose(LightScr);
	CBInstance.Light.Dir = LightDirNored;
	CBInstance.Light.Color = Scene->GetDirectionLight().Color;
	CBInstance.Light.Intensity = Scene->GetDirectionLight().Intensity;

	GDynamicRHI->WriteConstantBuffer(FrameRes.GetLightCB().get(), reinterpret_cast<void*>(&CBInstance), sizeof(LightCB));
}

void FFrameResourceManager::InitPaletteConstantBuffer(FScene* Scene, FFrameResource& FrameRes)
{
	FrameRes.SetPaletteCB(GDynamicRHI->CreateConstantBuffer(4352));
}

void FFrameResourceManager::CreateMapsForShadow(FFrameResource& FrameRes)
{
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

	TStaticMeshComponent Component = FAssetManager::Get()->CreateStaticMeshComponent(TriangleVertice, Indice,
		{ { 1.f, 1.f, 1.f }, FQuat(0, 0, 0, 1), { 0.f, 0.f, 0.f } }); // didnt use triangle's transform

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back( { "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 } );
	InputLayer.Elements.push_back( { "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 } );
	InputLayer.Elements.push_back( { "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 } );
	InputLayer.Elements.push_back( { "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 } );

	FrameRes.SetPostProcessTriangle(GDynamicRHI->CreateMesh(Component, InputLayer));
	FrameRes.SetPostProcessTriangleRes(make_shared<FMeshRes>());
}

void FFrameResourceManager::CreatePostProcessMaterials(FFrameResource& FrameRes)
{
	// create postprocess material
	// bloom setup
	vector<shared_ptr<FHandle>> TexHandles;
	TexHandles.push_back(FrameRes.GetSceneColorMap()->SrvHandle);
	FrameRes.BloomSetupMat = GDynamicRHI->CreateMaterial(L"Resource\\BloomSetup.hlsl", 256, TexHandles);

	// bloom down
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomSetupMap()->SrvHandle);
	FrameRes.BloomDownMat[0] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[0]->SrvHandle);
	FrameRes.BloomDownMat[1] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[1]->SrvHandle);
	FrameRes.BloomDownMat[2] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[2]->SrvHandle);
	FrameRes.BloomDownMat[3] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	// bloom up
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[2]->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[3]->SrvHandle);
	FrameRes.BloomUpMat[0] = GDynamicRHI->CreateMaterial(L"Resource\\BloomUpMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[1]->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomUpMapArray()[0]->SrvHandle);
	FrameRes.BloomUpMat[1] = GDynamicRHI->CreateMaterial(L"Resource\\BloomUpMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomDownMapArray()[0]->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomUpMapArray()[1]->SrvHandle);
	FrameRes.BloomUpMat[2] = GDynamicRHI->CreateMaterial(L"Resource\\BloomUpMat.hlsl", 256, TexHandles);

	// sun merge
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetBloomSetupMap()->SrvHandle);
	TexHandles.push_back(FrameRes.GetBloomUpMapArray()[2]->SrvHandle);
	FrameRes.SunMergeMat = GDynamicRHI->CreateMaterial(L"Resource\\SunMerge.hlsl", 256, TexHandles);

	// tonemapping
	TexHandles.clear();
	TexHandles.push_back(FrameRes.GetSceneColorMap()->SrvHandle);
	TexHandles.push_back(FrameRes.GetSunMergeMap()->SrvHandle);
	FrameRes.ToneMappingMat = GDynamicRHI->CreateMaterial(L"Resource\\ToneMapping.hlsl", 256, TexHandles);
}

void FFrameResourceManager::InitPostProcessConstantBuffer(FFrameResource& FrameRes)
{
	// update postprocess
	FVector2 WidthAndHeight = FVector2(static_cast<float>(GDynamicRHI->GetWidth()), static_cast<float>(GDynamicRHI->GetHeight()));

	// bloom setup
	struct FBloomSetupCB
	{
		FVector4 BufferSizeAndInvSize;
		float BloomThreshold;
	} BloomSetupStruct;
	BloomSetupStruct.BufferSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / 4.f);
	BloomSetupStruct.BloomThreshold = 1.0f;
	GDynamicRHI->WriteConstantBuffer(FrameRes.BloomSetupMat->CB.get(), reinterpret_cast<void*>(&BloomSetupStruct), sizeof(BloomSetupStruct));

	// bloom down
	for (uint32 i = 0; i < 4; i++)
	{
		struct FBloomDownCB
		{
			FVector4 BufferSizeAndInvSize;
			float BloomDownScale;
		} BloomDwonStruct;
		BloomDwonStruct.BufferSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / (4.f * static_cast<float>(pow(2, i + 1))));
		BloomDwonStruct.BloomDownScale = 0.66f * 4.0f;
		GDynamicRHI->WriteConstantBuffer(FrameRes.BloomDownMat[i]->CB.get(), reinterpret_cast<void*>(&BloomDwonStruct), sizeof(BloomDwonStruct));
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
		struct FBloomUpCB
		{
			FVector4 BufferASizeAndInvSize;
			FVector4 BufferBSizeAndInvSize;
			FVector4 BloomTintA;
			FVector4 BloomTintB;
			FVector2 BloomUpScales;
		} BloomUpStruct;
		BloomUpStruct.BufferASizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / (4.f * static_cast<float>(pow(2, i + 1))));
		BloomUpStruct.BufferBSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / (4.f * static_cast<float>(pow(2, i + 2))));
		BloomUpStruct.BloomTintA = BloomTintAs[2 - i] * (1.0f / 8.0f);
		BloomUpStruct.BloomTintB = BloomTintBs[2 - i] * (1.0f / 8.0f);
		BloomUpStruct.BloomUpScales.x = 0.66f * 2.0f;
		BloomUpStruct.BloomUpScales.y = 0.66f * 2.0f;
		GDynamicRHI->WriteConstantBuffer(FrameRes.BloomUpMat[2 - i]->CB.get(), reinterpret_cast<void*>(&BloomUpStruct), sizeof(BloomUpStruct));
	}

	// sun merge
	struct FSunMergeCB
	{
		FVector4 BloomUpSizeAndInvSize;
		FVector BloomColor;
	} SunMergeStruct;
	SunMergeStruct.BloomUpSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / 8.f);
	SunMergeStruct.BloomColor = FVector(BloomTint1) * BloomIntensity * 0.5f;
	GDynamicRHI->WriteConstantBuffer(FrameRes.SunMergeMat->CB.get(), reinterpret_cast<void*>(&SunMergeStruct), sizeof(SunMergeStruct));

}

void FFrameResourceManager::CreatePostProcessPipelines(FFrameResource& FrameRes)
{
	FMesh* Tri = FrameRes.GetPostProcessTriangle().get();

	// pipeline
	// bloom setup
	FShaderInputLayer ShaderInputLayer;
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	FrameRes.BloomSetupPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, FrameRes.BloomSetupMat.get());

	// bloom down
	FrameRes.BloomDownPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, FrameRes.BloomDownMat[0].get());

	// bloom up
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	FrameRes.BloomUpPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, FrameRes.BloomUpMat[0].get());

	// sun merge
	FrameRes.SunMergePipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R11G11B10_FLOAT, 1, Tri->InputLayer, ShaderInputLayer, FrameRes.SunMergeMat.get());

	// tone mapping
	ShaderInputLayer.Elements.clear();
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	ShaderInputLayer.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	FrameRes.ToneMappingPipeline = GDynamicRHI->CreatePipeline(FFormat::FORMAT_R8G8B8A8_UNORM, 1, Tri->InputLayer, ShaderInputLayer, FrameRes.ToneMappingMat.get());

}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, const uint32& FrameIndex)
{
	FFrameResource& FrameRes = FrameResArray[FrameIndex];

	// camera
	if (Scene->GetCurrentCamera().IsChanged == true)
	{
		FMatrix CamView = Scene->GetCurrentCamera().GetViewMatrix();
		FMatrix CamProj = Scene->GetCurrentCamera().GetPerspProjMatrix(1.0f, 3000.0f);
		FMatrix CamVP = glm::transpose(CamProj * CamView);
		FVector4 Eye(Scene->GetCurrentCamera().GetPosition().x, Scene->GetCurrentCamera().GetPosition().y, Scene->GetCurrentCamera().GetPosition().z, 1.f);

		struct CameraCB
		{
			FMatrix CamVP;
			FVector4 Eye;
		} CBInstance = { CamVP, Eye };

		GDynamicRHI->WriteConstantBuffer(FrameRes.GetCameraCB().get(), reinterpret_cast<void*>(&CBInstance), sizeof(CameraCB));
		Scene->GetCurrentCamera().IsChanged = false;
	}

	// character position
	auto MeshComponent = Scene->GetCharacter()->GetSkeletalMeshCom();
	FMatrix WorldMatrix = transpose(translate(MeshComponent->GetTransform().Translation) * toMat4(MeshComponent->GetTransform().Quat) * scale(MeshComponent->GetTransform().Scale));
	GDynamicRHI->WriteConstantBuffer(FrameRes.GetSkeletalMesh().MeshRes->SceneColorMat->CB.get(), reinterpret_cast<void*>(&WorldMatrix), sizeof(FMatrix));

	// animation
	struct PaletteCB
	{
		array<FMatrix, 68> GBoneTransforms;
	} CBInstance;

	for (uint32 i = 0; i < 68; i++)
	{
		CBInstance.GBoneTransforms[i] = glm::transpose(Scene->GetCharacter()->GetSkeletalMeshCom()->GetAnimator().GetPalette()[i]);
	}

	GDynamicRHI->WriteConstantBuffer(FrameRes.GetPaletteCB().get(), reinterpret_cast<void*>(&CBInstance), sizeof(PaletteCB));
}