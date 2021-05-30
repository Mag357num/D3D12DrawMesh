#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include "RHIResource.h"
#include "DX12Resource.h"
#include "MathExtend.h"
#include "AssetManager.h"

void FFrameResourceManager::InitFrameResource(TScene* Scene, const uint32& FrameCount)
{
	// single buffer frame resource
	InitLightConstantBuffer( Scene, SingleBufferFrameRes );
	CreateSamplers();
	CreatePostProcessTriangle();

	// multi buffer frame resource
	MultiBufferFrameRes.resize( FrameCount );
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		FMultiBufferFrameResource& DFrameRes = MultiBufferFrameRes[FrameIndex];

		InitCameraConstantBuffer(Scene, DFrameRes);
		InitCharacterPaletteConstantBuffer(Scene, DFrameRes);
		CreateMapsForShadow(DFrameRes);
		CreateMapsForScene(DFrameRes);
		CreateMapsForPostProcess(DFrameRes);

		// TODO: move to single buffer frame resource
		CreatePostProcessMaterials( DFrameRes );
		InitPostProcessConstantBuffer( DFrameRes );
		CreatePostProcessPipelines( DFrameRes );
	}
}

void FFrameResourceManager::CreateFrameResourcesFromScene(const shared_ptr<TScene> Scene, const uint32& FrameNum)
{
	// TODO: param hard coding
	const wstring Shader_ShadowPass_StaticMesh = L"Resource\\ShadowMapping_StaticMesh.hlsl";
	const wstring Shader_ShadowPass_SkeletalMesh = L"Resource\\ShadowMapping_SkeletalMesh.hlsl";
	const wstring Shader_ScenePass_StaticMesh = L"Resource\\SceneColor_StaticMesh.hlsl";
	const wstring Shader_ScenePass_SkeletalMesh = L"Resource\\SceneColor_SkeletalMesh.hlsl";

	const uint32 CbSize_ShadowPass_StaticMesh = 256;
	const uint32 CbSize_ShadowPass_SkeletalMesh = 256;
	const uint32 CbSize_ScenePass_StaticMesh = 256;
	const uint32 CbSize_ScenePass_SkeletalMesh = 256;

	FVertexInputLayer VIL_StaticMesh;
	VIL_StaticMesh.Elements.push_back( { "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 } );
	VIL_StaticMesh.Elements.push_back( { "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 } );
	VIL_StaticMesh.Elements.push_back( { "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 } );
	VIL_StaticMesh.Elements.push_back( { "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 } );

	FVertexInputLayer VIL_SkeletalMesh;
	VIL_SkeletalMesh.Elements.push_back( { "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 } );
	VIL_SkeletalMesh.Elements.push_back( { "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 } );
	VIL_SkeletalMesh.Elements.push_back( { "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 } );
	VIL_SkeletalMesh.Elements.push_back( { "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 } );
	VIL_SkeletalMesh.Elements.push_back( { "WEIGHTS", 0, FFormat::FORMAT_R16G16B16A16_UINT, 0, 48, 0, 0 } ); // array<uint16, 4>
	VIL_SkeletalMesh.Elements.push_back( { "BONEINDICES", 0, FFormat::FORMAT_R16G16B16A16_UINT, 0, 56, 0, 0 } ); // array<uint16, 4>

	FShaderInputLayer SIL_ShadowPass_StaticMesh;
	SIL_ShadowPass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ShadowPass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );

	FShaderInputLayer SIL_ScenePass_StaticMesh;
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );

	FShaderInputLayer SIL_ShadowPass_SkeletalMesh;
	SIL_ShadowPass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ShadowPass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ShadowPass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL } );

	FShaderInputLayer SIL_ScenePass_SkeletalMesh;
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );


	RHI::GDynamicRHI->BegineCreateResource();
	InitFrameResource(Scene.get(), FrameNum);

	// character
	SingleBufferFrameRes.CharacterMesh = GDynamicRHI->CreateMesh_new( *Scene->GetCharacter()->GetSkeletalMeshCom() );

	// static frame resource
	for (uint32 i = 0; i < static_cast<uint32>(Scene->GetStaticMeshActors().size()); ++i)
	{
		SingleBufferFrameRes.StaticMeshes.push_back( GDynamicRHI->CreateMesh_new( *Scene->GetStaticMeshActors()[i].GetComs()[0]->As<TStaticMeshComponent>() ) );
	}

	// dynamic frame resource
	for (uint32 FrameIndex = 0; FrameIndex < FrameNum; ++FrameIndex)
	{
		FMultiBufferFrameResource& DFrameRes = MultiBufferFrameRes[FrameIndex];

		// character mesh render resource
		{
			shared_ptr<FRenderResource_new> RR_ShadowPass_SkeletalMesh = CreateRenderResource
			(
				Shader_ShadowPass_SkeletalMesh,
				CbSize_ShadowPass_SkeletalMesh,
				VIL_SkeletalMesh,
				SIL_ShadowPass_SkeletalMesh,
				FFormat::FORMAT_UNKNOWN,
				0
			);
			FTransform& Trans = Scene->GetCharacter()->GetSkeletalMeshCom()->GetTransform();
			FMatrix WVP = transpose( Scene->GetDirectionLight().GetLightVPMatrix() * translate( Trans.Translation ) * toMat4( Trans.Quat ) * scale( Trans.Scale ) );
			GDynamicRHI->WriteConstantBuffer( RR_ShadowPass_SkeletalMesh->CB, );
			DFrameRes.RR_ShadowPass.push_back( RR_ShadowPass_SkeletalMesh ); // store in DFrameRes
			SingleBufferFrameRes.CharacterMesh->RR_ShadowPass.push_back( RR_ShadowPass_SkeletalMesh.get() ); // leave a ref in mesh


			shared_ptr<FRenderResource_new> RR_ScenePass_SkeletalMesh = CreateRenderResource
			(
				Shader_ScenePass_SkeletalMesh,
				CbSize_ScenePass_SkeletalMesh,
				VIL_SkeletalMesh,
				SIL_ScenePass_SkeletalMesh,
				FFormat::FORMAT_R16G16B16A16_FLOAT,
				1
			);
			DFrameRes.RR_ScenePass.push_back( RR_ScenePass_SkeletalMesh );
			SingleBufferFrameRes.CharacterMesh->RR_ScenePass.push_back( RR_ScenePass_SkeletalMesh.get() );
		}


		// static mesh render resource
		for (uint32 i = 0; i < static_cast<uint32>(Scene->GetStaticMeshActors().size()); ++i)
		{
			shared_ptr<FRenderResource_new> RR_ShadowPass_StaticMesh = CreateRenderResource
			(
				Shader_ShadowPass_StaticMesh,
				CbSize_ShadowPass_StaticMesh,
				VIL_StaticMesh,
				SIL_ShadowPass_StaticMesh,
				FFormat::FORMAT_UNKNOWN,
				0
			);
			DFrameRes.RR_ShadowPass.push_back(RR_ShadowPass_StaticMesh);
			SingleBufferFrameRes.StaticMeshes[i]->RR_ShadowPass.push_back( RR_ShadowPass_StaticMesh.get() );

			shared_ptr<FRenderResource_new> RR_ScenePass_StaticMesh = CreateRenderResource
			(
				Shader_ScenePass_StaticMesh,
				CbSize_ScenePass_StaticMesh,
				VIL_StaticMesh,
				SIL_ScenePass_StaticMesh,
				FFormat::FORMAT_R16G16B16A16_FLOAT,
				1
			);
			DFrameRes.RR_ScenePass.push_back(RR_ScenePass_StaticMesh);
			SingleBufferFrameRes.StaticMeshes[i]->RR_ScenePass.push_back( RR_ScenePass_StaticMesh.get() );
		}
	}

	InitFMeshConstantBuffer();
	RHI::GDynamicRHI->EndCreateResource();
}

shared_ptr<RHI::FRenderResource_new> FFrameResourceManager::CreateRenderResource( const wstring& Shader, const uint32& Size, FVertexInputLayer VIL, FShaderInputLayer SIL, FFormat RtFormat, uint32 RtNum )
{
	shared_ptr<FRenderResource_new> RR = make_shared<FRenderResource_new>();
	RR->VS = GDynamicRHI->CreateVertexShader( Shader );
	RR->PS = GDynamicRHI->CreatePixelShader( Shader );
	RR->CB = GDynamicRHI->CreateConstantBuffer( Size );
	RR->Sig = GDynamicRHI->CreateRootSignatrue( SIL );
	RR->PSO = GDynamicRHI->CreatePso( RtFormat, VIL, RtNum, RR->VS.get(), RR->PS.get(), RR->Sig.get() );

	return RR;
}

FFrameMesh_deprecated FFrameResourceManager::CreateFrameMesh_deprecated(TStaticMeshComponent& MeshComponent)
{
	FFrameMesh_deprecated MeshComFrameRes;

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back({ "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 });
	InputLayer.Elements.push_back({ "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 });
	InputLayer.Elements.push_back({ "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 });
	InputLayer.Elements.push_back({ "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 });

	MeshComFrameRes.Mesh = GDynamicRHI->CreateMesh_deprecated(MeshComponent, InputLayer);
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

FFrameMesh_deprecated FFrameResourceManager::CreateFrameMesh_deprecated(TSkeletalMeshComponent& MeshComponent)
{
	FFrameMesh_deprecated MeshComFrameRes;

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back({ "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 });
	InputLayer.Elements.push_back({ "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 });
	InputLayer.Elements.push_back({ "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 });
	InputLayer.Elements.push_back({ "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 });
	InputLayer.Elements.push_back({ "WEIGHTS", 0, FFormat::FORMAT_R16G16B16A16_UINT, 0, 48, 0, 0 }); // array<uint16, 4>
	InputLayer.Elements.push_back({ "BONEINDICES", 0, FFormat::FORMAT_R16G16B16A16_UINT, 0, 56, 0, 0 }); // array<uint16, 4>

	MeshComFrameRes.Mesh = GDynamicRHI->CreateMesh_deprecated(MeshComponent, InputLayer);
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

void FFrameResourceManager::InitCameraConstantBuffer(TScene* Scene, FMultiBufferFrameResource& FrameRes)
{
	FrameRes.CameraCB = GDynamicRHI->CreateConstantBuffer(256);

	FMatrix CamView = Scene->GetCurrentCamera()->GetViewMatrix();
	FMatrix CamProj = Scene->GetCurrentCamera()->GetPerspProjMatrix(1.0f, 3000.0f);
	FMatrix CamVP = glm::transpose(CamProj * CamView);
	FVector4 Eye(Scene->GetCurrentCamera()->GetPosition().x, Scene->GetCurrentCamera()->GetPosition().y, Scene->GetCurrentCamera()->GetPosition().z, 1.f);

	struct CameraCB
	{
		FMatrix CamVP;
		FVector4 Eye;
	} CBInstance = { CamVP, Eye };

	GDynamicRHI->WriteConstantBuffer(FrameRes.CameraCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CameraCB));
}

void FFrameResourceManager::InitLightConstantBuffer(TScene* Scene, FSingleBufferFrameResource& FrameRes)
{
	FrameRes.StaticSkyLightCB = GDynamicRHI->CreateConstantBuffer(256);

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

	CBInstance.VPMatrix = glm::transpose(Scene->GetDirectionLight().GetLightVPMatrix());
	CBInstance.ScreenMatrix = glm::transpose(LightScr);
	CBInstance.Light = Scene->GetDirectionLight();
	CBInstance.Light.Dir = glm::normalize( CBInstance.Light.Dir );

	GDynamicRHI->WriteConstantBuffer(FrameRes.StaticSkyLightCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(LightCB));
}

void FFrameResourceManager::InitCharacterPaletteConstantBuffer(TScene* Scene, FMultiBufferFrameResource& FrameRes)
{
	FrameRes.CharacterPaletteCB = GDynamicRHI->CreateConstantBuffer(4352);
}

void FFrameResourceManager::CreateMapsForShadow(FMultiBufferFrameResource& FrameRes)
{
	// create and commit shadow map
	FrameRes.ShadowMap = GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP_TT, FrameRes.ShadowMapSize, FrameRes.ShadowMapSize);
	GDynamicRHI->CommitTextureAsView(FrameRes.ShadowMap.get(), FResViewType::DSV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.ShadowMap.get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreateSamplers()
{
	// create and commit sampler
	SingleBufferFrameRes.ClampSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP_ST);
	SingleBufferFrameRes.WarpSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::WARP_ST);
}

void FFrameResourceManager::CreateMapsForScene(FMultiBufferFrameResource& FrameRes)
{
	// create and commit Ds map
	FrameRes.DepthStencilMap = GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight());
	GDynamicRHI->CommitTextureAsView(FrameRes.DepthStencilMap.get(), FResViewType::DSV_RVT);

	// create and commit scene color
	FrameRes.SceneColorMap = GDynamicRHI->CreateTexture(FTextureType::SCENE_COLOR_TT, GDynamicRHI->GetWidth(), GDynamicRHI->GetHeight());
	GDynamicRHI->CommitTextureAsView(FrameRes.SceneColorMap.get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.SceneColorMap.get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreateMapsForPostProcess(FMultiBufferFrameResource& FrameRes)
{
	// create and commit bloom down and up texture
	FrameRes.BloomSetupMap = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4);
	GDynamicRHI->CommitTextureAsView(FrameRes.BloomSetupMap.get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.BloomSetupMap.get(), FResViewType::SRV_RVT);

	for (uint32 i = 0; i < 4; i++)
	{
		uint32 ShrinkTimes = static_cast<uint32>(pow(2, 3 + i));
		FrameRes.BloomDownMapArray.push_back(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / ShrinkTimes, GDynamicRHI->GetHeight() / ShrinkTimes));
		GDynamicRHI->CommitTextureAsView(FrameRes.BloomDownMapArray[i].get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameRes.BloomDownMapArray[i].get(), FResViewType::SRV_RVT);
	}

	for (int i = 4; i > 1; i--)
	{
		uint32 ShrinkTimes = static_cast<uint32>(pow(2, 1 + i));
		FrameRes.BloomUpMapArray.push_back(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / ShrinkTimes, GDynamicRHI->GetHeight() / ShrinkTimes));
		GDynamicRHI->CommitTextureAsView(FrameRes.BloomUpMapArray[4 - i].get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(FrameRes.BloomUpMapArray[4 - i].get(), FResViewType::SRV_RVT);
	}

	FrameRes.SunMergeMap = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GDynamicRHI->GetWidth() / 4, GDynamicRHI->GetHeight() / 4);
	GDynamicRHI->CommitTextureAsView(FrameRes.SunMergeMap.get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(FrameRes.SunMergeMap.get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreatePostProcessTriangle()
{
	// create postprocess mesh and mesh resource
	vector<TStaticVertex> TriangleVertice;
	TriangleVertice.push_back(TStaticVertex(FVector(1.f, -1.f, 0.0f), FVector(1, 1, 1), FVector2(1.f, 1.f), FVector4(1, 1, 1, 1)));
	TriangleVertice.push_back(TStaticVertex(FVector(1.f, 3.f, 0.0f), FVector(1, 1, 1), FVector2(1.f, -1.f), FVector4(1, 1, 1, 1)));
	TriangleVertice.push_back(TStaticVertex(FVector(-3.f, -1.f, 0.0f), FVector(1, 1, 1), FVector2(-1.f, 1.f), FVector4(1, 1, 1, 1)));

	vector<uint32> Indice = { 0, 1, 2 };

	TStaticMeshComponent Component = FAssetManager::Get()->CreateStaticMeshComponent(TriangleVertice, Indice,
		{ { 1.f, 1.f, 1.f }, FQuat(0, 0, 0, 1), { 0.f, 0.f, 0.f } }); // didnt use triangle's transform

	FVertexInputLayer InputLayer;
	InputLayer.Elements.push_back( { "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 } );
	InputLayer.Elements.push_back( { "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 } );
	InputLayer.Elements.push_back( { "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 } );
	InputLayer.Elements.push_back( { "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 } );

	SingleBufferFrameRes.PostProcessTriangle = GDynamicRHI->CreateMesh_deprecated(Component, InputLayer);
	SingleBufferFrameRes.PostProcessTriangleRes = make_shared<FMeshRes>();
}

void FFrameResourceManager::CreatePostProcessMaterials(FMultiBufferFrameResource& FrameRes)
{
	// create postprocess material
	// bloom setup
	vector<shared_ptr<FHandle>> TexHandles;
	TexHandles.push_back(FrameRes.SceneColorMap->SrvHandle);
	FrameRes.BloomSetupMat = GDynamicRHI->CreateMaterial(L"Resource\\BloomSetup.hlsl", 256, TexHandles);

	// bloom down
	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomSetupMap->SrvHandle);
	FrameRes.BloomDownMat[0] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomDownMapArray[0]->SrvHandle);
	FrameRes.BloomDownMat[1] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomDownMapArray[1]->SrvHandle);
	FrameRes.BloomDownMat[2] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomDownMapArray[2]->SrvHandle);
	FrameRes.BloomDownMat[3] = GDynamicRHI->CreateMaterial(L"Resource\\BloomDownMat.hlsl", 256, TexHandles);

	// bloom up
	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomDownMapArray[2]->SrvHandle);
	TexHandles.push_back(FrameRes.BloomDownMapArray[3]->SrvHandle);
	FrameRes.BloomUpMat[0] = GDynamicRHI->CreateMaterial(L"Resource\\BloomUpMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomDownMapArray[1]->SrvHandle);
	TexHandles.push_back(FrameRes.BloomUpMapArray[0]->SrvHandle);
	FrameRes.BloomUpMat[1] = GDynamicRHI->CreateMaterial(L"Resource\\BloomUpMat.hlsl", 256, TexHandles);

	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomDownMapArray[0]->SrvHandle);
	TexHandles.push_back(FrameRes.BloomUpMapArray[1]->SrvHandle);
	FrameRes.BloomUpMat[2] = GDynamicRHI->CreateMaterial(L"Resource\\BloomUpMat.hlsl", 256, TexHandles);

	// sun merge
	TexHandles.clear();
	TexHandles.push_back(FrameRes.BloomSetupMap->SrvHandle);
	TexHandles.push_back(FrameRes.BloomUpMapArray[2]->SrvHandle);
	FrameRes.SunMergeMat = GDynamicRHI->CreateMaterial(L"Resource\\SunMerge.hlsl", 256, TexHandles);

	// tonemapping
	TexHandles.clear();
	TexHandles.push_back(FrameRes.SceneColorMap->SrvHandle);
	TexHandles.push_back(FrameRes.SunMergeMap->SrvHandle);
	FrameRes.ToneMappingMat = GDynamicRHI->CreateMaterial(L"Resource\\ToneMapping.hlsl", 256, TexHandles);
}

void FFrameResourceManager::InitPostProcessConstantBuffer(FMultiBufferFrameResource& FrameRes)
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

void FFrameResourceManager::CreatePostProcessPipelines(FMultiBufferFrameResource& FrameRes)
{
	FMesh_deprecated* Tri = SingleBufferFrameRes.PostProcessTriangle.get();

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

void FFrameResourceManager::UpdateFrameResources(TScene* Scene, const uint32& FrameIndex)
{
	FMultiBufferFrameResource& FrameRes = MultiBufferFrameRes[FrameIndex];

	// camera
	if (Scene->GetCurrentCamera()->IsChanged == true)
	{
		FMatrix CamView = Scene->GetCurrentCamera()->GetViewMatrix();
		FMatrix CamProj = Scene->GetCurrentCamera()->GetPerspProjMatrix(1.0f, 3000.0f);
		FMatrix CamVP = glm::transpose(CamProj * CamView);
		FVector4 Eye(Scene->GetCurrentCamera()->GetPosition().x, Scene->GetCurrentCamera()->GetPosition().y, Scene->GetCurrentCamera()->GetPosition().z, 1.f);

		struct CameraCB
		{
			FMatrix CamVP;
			FVector4 Eye;
		} CBInstance = { CamVP, Eye };

		GDynamicRHI->WriteConstantBuffer(FrameRes.CameraCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CameraCB));
		Scene->GetCurrentCamera()->IsChanged = false;
	}

	// character position
	auto MeshComponent = Scene->GetCharacter()->GetSkeletalMeshCom();
	FMatrix WorldMatrix = transpose(translate(MeshComponent->GetTransform().Translation) * toMat4(MeshComponent->GetTransform().Quat) * scale(MeshComponent->GetTransform().Scale));
	GDynamicRHI->WriteConstantBuffer(FrameRes.SkeletalMesh.MeshRes->SceneColorMat->CB.get(), reinterpret_cast<void*>(&WorldMatrix), sizeof(FMatrix));

	// animation
	struct PaletteCB
	{
		array<FMatrix, 68> GBoneTransforms;
	} CBInstance;

	for (uint32 i = 0; i < 68; i++)
	{
		CBInstance.GBoneTransforms[i] = glm::transpose(Scene->GetCharacter()->GetSkeletalMeshCom()->GetAnimator().GetPalette()[i]);
	}

	GDynamicRHI->WriteConstantBuffer(FrameRes.CharacterPaletteCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(PaletteCB));
}