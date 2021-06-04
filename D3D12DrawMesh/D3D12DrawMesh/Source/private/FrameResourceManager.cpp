#include "FrameResourceManager.h"
#include "DynamicRHI.h"
#include "RHIResource.h"
#include "DX12Resource.h"
#include "MathExtend.h"
#include "AssetManager.h"
#include "Engine.h"
#include "Character.h"

void FFrameResourceManager::InitFrameResource(FScene* Scene, const uint32& FrameCount)
{
	// single buffer frame resource
	CreateDirectionalLightCB( Scene, SFrameRes );
	CreateSamplers();
	CreatePPTriangle();
	CreatePPTriangleRR();
	CreateMapsForShadow();
	CreateMapsForScene();
	CreateMapsForPostProcess();

	// multi buffer frame resource
	MFrameRes.resize( FrameCount );
	for (uint32 FrameIndex = 0; FrameIndex < FrameCount; ++FrameIndex)
	{
		CreateCameraCB(Scene, MFrameRes[FrameIndex]);
		CreateCharacterPaletteCB(Scene, MFrameRes[FrameIndex]);
	}
}

void FFrameResourceManager::CreateFrameResOfSceneActors(const shared_ptr<FScene> Scene, const uint32& FrameNum)
{
	// TODO: param hard coding
	 FFormat SceneMapFormat = GEngine->UsePostProcess() ? FFormat::FORMAT_R16G16B16A16_FLOAT : FFormat::FORMAT_R8G8B8A8_UNORM;

	const wstring Shader_ShadowPass_StaticMesh = L"Resource\\ShadowMapping_StaticMesh.hlsl";
	const wstring Shader_ScenePass_StaticMesh = L"Resource\\SceneColor_StaticMesh.hlsl";
	const wstring Shader_ShadowPass_SkeletalMesh = L"Resource\\ShadowMapping_SkeletalMesh.hlsl";
	const wstring Shader_ScenePass_SkeletalMesh = L"Resource\\SceneColor_SkeletalMesh.hlsl";
	const wstring Shader_ScenePass_LightSource = L"Resource\\SceneColor_DirectionLightSource.hlsl";

	const uint32 CbSize_ShadowPass_StaticMesh = 256;
	const uint32 CbSize_ScenePass_StaticMesh = 256;
	const uint32 CbSize_ShadowPass_SkeletalMesh = 256;
	const uint32 CbSize_ScenePass_SkeletalMesh = 256;
	const uint32 CbSize_ScenePass_LightSource = 256;

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

	FShaderInputLayer SIL_ScenePass_LightSource;
	SIL_ScenePass_LightSource.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });

	FShaderInputLayer SIL_ShadowPass_StaticMesh;
	SIL_ShadowPass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );

	FShaderInputLayer SIL_ScenePass_StaticMesh;
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );
	SIL_ScenePass_StaticMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );

	FShaderInputLayer SIL_ShadowPass_SkeletalMesh;
	SIL_ShadowPass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ShadowPass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );

	FShaderInputLayer SIL_ScenePass_SkeletalMesh;
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, FShaderVisibility::SHADER_VISIBILITY_ALL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );
	SIL_ScenePass_SkeletalMesh.Elements.push_back( { FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL } );

	const FMatrix& LV = Scene->GetDirectionalLight()->GetViewMatrix_RenderThread();
	const FMatrix& LO = Scene->GetDirectionalLight()->GetOMatrix_RenderThread();
	const FMatrix& CV = Scene->GetCurrentCamera()->GetViewMatrix_RenderThread();
	const FMatrix& CP = Scene->GetCurrentCamera()->GetPerspProjMatrix_RenderThread();

	RHI::GDynamicRHI->BegineCreateResource();
	InitFrameResource(Scene.get(), FrameNum);

	// direction light source
	{
		// mesh
		SFrameRes.DirectionalLight = GDynamicRHI->CreateGeometry(*Scene->GetDirectionalLight()->GetStaticMeshComponent());
		// need no shadow pass
		// scenepass rr
		auto RR_ScenePass = CreateRenderResource
		(
			Shader_ScenePass_LightSource,
			CbSize_ScenePass_LightSource,
			VIL_StaticMesh,
			SIL_ScenePass_LightSource,
			SceneMapFormat,
			1,
			GDynamicRHI->GetFrameCount()
		);
		FMatrix WVP = transpose(CP * CV * Scene->GetDirectionalLight()->GetStaticMeshComponent()->GetWorldMatrix());
		for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
		{
			GDynamicRHI->WriteConstantBuffer(RR_ScenePass->CBs[i].get(), reinterpret_cast<void*>(&WVP), sizeof(WVP));
		}
		SFrameRes.RRMap_ScenePass.insert({ SFrameRes.DirectionalLight.get(), RR_ScenePass });
	}

	// point light source
	{
		for (uint32 i = 0; i < static_cast<uint32>(Scene->GetPointLights().size()); ++i)
		{
			// mesh
			SFrameRes.PointLights.push_back(GDynamicRHI->CreateGeometry(*Scene->GetPointLights()[i]->GetStaticMeshComponent()));
			// need no shadow pass
			// scenepass rr
			auto RR_ScenePass = CreateRenderResource
			(
				Shader_ScenePass_LightSource,
				CbSize_ScenePass_LightSource,
				VIL_StaticMesh,
				SIL_ScenePass_LightSource,
				SceneMapFormat,
				1,
				GDynamicRHI->GetFrameCount()
			);
			FMatrix WVP = transpose(CP * CV * Scene->GetPointLights()[i]->GetStaticMeshComponent()->GetWorldMatrix());
			for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
			{
				GDynamicRHI->WriteConstantBuffer(RR_ScenePass->CBs[i].get(), reinterpret_cast<void*>(&WVP), sizeof(WVP));
			}
			SFrameRes.RRMap_ScenePass.insert({ SFrameRes.PointLights[i].get(), RR_ScenePass });
		}
	}

	// character
	{
		// mesh
		SFrameRes.CharacterMesh = GDynamicRHI->CreateGeometry(*Scene->GetCurrentCharacter()->GetSkeletalMeshCom());
		// shadowpass rr
		auto RR_ShadowPass = CreateRenderResource
		(
			Shader_ShadowPass_SkeletalMesh,
			CbSize_ShadowPass_SkeletalMesh,
			VIL_SkeletalMesh,
			SIL_ShadowPass_SkeletalMesh,
			FFormat::FORMAT_UNKNOWN,
			0,
			GDynamicRHI->GetFrameCount()
		);
		FMatrix WVO = transpose(LO * LV * Scene->GetCurrentCharacter()->GetSkeletalMeshCom()->GetWorldMatrix());
		for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
		{
			GDynamicRHI->WriteConstantBuffer(RR_ShadowPass->CBs[i].get(), reinterpret_cast<void*>(&WVO), sizeof(WVO));
		}
		SFrameRes.RRMap_ShadowPass.insert({ SFrameRes.CharacterMesh.get(), RR_ShadowPass });

		// scenepass rr
		auto RR_ScenePass = CreateRenderResource
		(
			Shader_ScenePass_SkeletalMesh,
			CbSize_ScenePass_SkeletalMesh,
			VIL_SkeletalMesh,
			SIL_ScenePass_SkeletalMesh,
			SceneMapFormat,
			1,
			GDynamicRHI->GetFrameCount()
		);
		FMatrix Wrold = transpose(Scene->GetCurrentCharacter()->GetSkeletalMeshCom()->GetWorldMatrix());
		for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
		{
			GDynamicRHI->WriteConstantBuffer(RR_ScenePass->CBs[i].get(), reinterpret_cast<void*>(&Wrold), sizeof(Wrold));
		}
		SFrameRes.RRMap_ScenePass.insert({ SFrameRes.CharacterMesh.get(), RR_ScenePass });
	}

	// static mesh
	{
		for (uint32 i = 0; i < static_cast<uint32>(Scene->GetStaticMeshActors().size()); ++i)
		{
			// mesh
			SFrameRes.StaticMeshes.push_back(GDynamicRHI->CreateGeometry(*Scene->GetStaticMeshActors()[i]->GetStaticMeshComponent()));
			// shadowpass rr
			auto RR_ShadowPass = CreateRenderResource
			(
				Shader_ShadowPass_StaticMesh,
				CbSize_ShadowPass_StaticMesh,
				VIL_StaticMesh,
				SIL_ShadowPass_StaticMesh,
				FFormat::FORMAT_UNKNOWN,
				0,
				GDynamicRHI->GetFrameCount()
			);
			FMatrix WVP = transpose(LO * LV * Scene->GetStaticMeshActors()[i]->GetStaticMeshComponent()->GetWorldMatrix());
			for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
			{
				GDynamicRHI->WriteConstantBuffer(RR_ShadowPass->CBs[i].get(), reinterpret_cast<void*>(&WVP), sizeof(WVP));
			}
			SFrameRes.RRMap_ShadowPass.insert({ SFrameRes.StaticMeshes[i].get(), RR_ShadowPass });

			// scenepass rr
			auto RR_ScenePass = CreateRenderResource
			(
				Shader_ScenePass_StaticMesh,
				CbSize_ScenePass_StaticMesh,
				VIL_StaticMesh,
				SIL_ScenePass_StaticMesh,
				SceneMapFormat,
				1,
				GDynamicRHI->GetFrameCount()
			);
			FMatrix Wrold = transpose(Scene->GetStaticMeshActors()[i]->GetStaticMeshComponent()->GetWorldMatrix());
			for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
			{
				GDynamicRHI->WriteConstantBuffer(RR_ScenePass->CBs[i].get(), reinterpret_cast<void*>(&Wrold), sizeof(Wrold));
			}
			SFrameRes.RRMap_ScenePass.insert({ SFrameRes.StaticMeshes[i].get(), RR_ScenePass });
		}
	}

	RHI::GDynamicRHI->EndCreateResource();
}

shared_ptr<RHI::FRenderResource> FFrameResourceManager::CreateRenderResource( const wstring& Shader, const uint32& Size, FVertexInputLayer VIL, FShaderInputLayer SIL, FFormat RtFormat, uint32 RtNum, uint32 FrameCount )
{
	shared_ptr<FRenderResource> RR = make_shared<FRenderResource>();
	RR->VS = GDynamicRHI->CreateVertexShader( Shader );
	RR->PS = GDynamicRHI->CreatePixelShader( Shader );
	RR->Sig = GDynamicRHI->CreateRootSignatrue( SIL );
	RR->PSO = GDynamicRHI->CreatePso( RtFormat, VIL, RtNum, RR->VS.get(), RR->PS.get(), RR->Sig.get() );
	for (uint32 i = 0; i < FrameCount; i++)
	{
		RR->CBs.push_back(GDynamicRHI->CreateConstantBuffer(Size)); // TODO: not all rr need 3 cb, for example bloomsetup
	}

	return RR;
}

void FFrameResourceManager::CreateCameraCB(FScene* Scene, FMultiBufferFrameResource& FrameRes)
{
	// create
	FrameRes.CameraCB = GDynamicRHI->CreateConstantBuffer(256);

	// init. if not init, will be blink
	FMatrix CamView = Scene->GetCurrentCamera()->GetViewMatrix_RenderThread();
	FMatrix CamProj = Scene->GetCurrentCamera()->GetPerspProjMatrix_RenderThread();
	FMatrix CamVP = glm::transpose(CamProj * CamView);
	const FVector& CamPos = Scene->GetCurrentCamera()->GetTransform().Translation;
	FVector4 Eye(CamPos.x, CamPos.y, CamPos.z, 1.f);
	struct CameraConstantBuffer
	{
		FMatrix CamVP;
		FVector4 Eye;
	} CBInstance = { CamVP, Eye };

	GDynamicRHI->WriteConstantBuffer(FrameRes.CameraCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CameraConstantBuffer));
}

void FFrameResourceManager::CreateDirectionalLightCB(FScene* Scene, FSingleBufferFrameResource& FrameRes)
{
	FrameRes.StaticDirectionalLightCB = GDynamicRHI->CreateConstantBuffer(256);
}

void FFrameResourceManager::CreateCharacterPaletteCB(FScene* Scene, FMultiBufferFrameResource& FrameRes)
{
	FrameRes.CharacterPaletteCB = GDynamicRHI->CreateConstantBuffer(4352); // TODO: hard coding
}

void FFrameResourceManager::CreateMapsForShadow()
{
	// create and commit shadow map
	SFrameRes.ShadowMap = GDynamicRHI->CreateTexture(FTextureType::SHADOW_MAP_TT, SFrameRes.ShadowMapSize, SFrameRes.ShadowMapSize);
	GDynamicRHI->CommitTextureAsView(SFrameRes.ShadowMap.get(), FResViewType::DSV_RVT);
	GDynamicRHI->CommitTextureAsView(SFrameRes.ShadowMap.get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreateSamplers()
{
	// create and commit sampler
	SFrameRes.ClampSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::CLAMP_ST);
	SFrameRes.WarpSampler = GDynamicRHI->CreateAndCommitSampler(FSamplerType::WARP_ST);
}

void FFrameResourceManager::CreateMapsForScene()
{
	// create and commit Ds map
	SFrameRes.DepthStencilMap = GDynamicRHI->CreateTexture(FTextureType::DEPTH_STENCIL_MAP_TT, GEngine->GetWidth(), GEngine->GetHeight());
	GDynamicRHI->CommitTextureAsView(SFrameRes.DepthStencilMap.get(), FResViewType::DSV_RVT);

	// create and commit scene color
	SFrameRes.SceneColorMap = GDynamicRHI->CreateTexture(FTextureType::SCENE_COLOR_TT, GEngine->GetWidth(), GEngine->GetHeight());
	GDynamicRHI->CommitTextureAsView(SFrameRes.SceneColorMap.get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(SFrameRes.SceneColorMap.get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreateMapsForPostProcess()
{
	// bloom set up
	SFrameRes.BloomSetupMap = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GEngine->GetWidth() / 4, GEngine->GetHeight() / 4);
	GDynamicRHI->CommitTextureAsView(SFrameRes.BloomSetupMap.get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(SFrameRes.BloomSetupMap.get(), FResViewType::SRV_RVT);

	// bloom down
	for (uint32 i = 0; i < 4; i++)
	{
		uint32 ShrinkTimes = static_cast<uint32>(pow(2, 3 + i));
		SFrameRes.BloomDownMapArray.push_back(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GEngine->GetWidth() / ShrinkTimes, GEngine->GetHeight() / ShrinkTimes));
		GDynamicRHI->CommitTextureAsView(SFrameRes.BloomDownMapArray[i].get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(SFrameRes.BloomDownMapArray[i].get(), FResViewType::SRV_RVT);
	}

	// bloom up
	for (uint32 i = 0; i < 3; i++)
	{
		uint32 ShrinkTimes = static_cast<uint32>(pow(2, 5 - i));
		SFrameRes.BloomUpMapArray.push_back(GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GEngine->GetWidth() / ShrinkTimes, GEngine->GetHeight() / ShrinkTimes));
		GDynamicRHI->CommitTextureAsView(SFrameRes.BloomUpMapArray[i].get(), FResViewType::RTV_RVT);
		GDynamicRHI->CommitTextureAsView(SFrameRes.BloomUpMapArray[i].get(), FResViewType::SRV_RVT);
	}

	// sun merge
	SFrameRes.SunMergeMap = GDynamicRHI->CreateTexture(FTextureType::RENDER_TARGET_TEXTURE_TT, GEngine->GetWidth() / 4, GEngine->GetHeight() / 4);
	GDynamicRHI->CommitTextureAsView(SFrameRes.SunMergeMap.get(), FResViewType::RTV_RVT);
	GDynamicRHI->CommitTextureAsView(SFrameRes.SunMergeMap.get(), FResViewType::SRV_RVT);
}

void FFrameResourceManager::CreatePPTriangle()
{
	// create postprocess mesh and mesh resource
	FStaticMeshLOD Lod;
	Lod.Vertice.push_back(FStaticVertex(FVector(1.f, -1.f, 0.0f), FVector(1.f, 1.f, 1.f), FVector2(1.f, 1.f), FVector4(1.f, 1.f, 1.f, 1.f)));
	Lod.Vertice.push_back(FStaticVertex(FVector(1.f, 3.f, 0.0f), FVector(1.f, 1.f, 1.f), FVector2(1.f, -1.f), FVector4(1.f, 1.f, 1.f, 1.f)));
	Lod.Vertice.push_back(FStaticVertex(FVector(-3.f, -1.f, 0.0f), FVector(1.f, 1.f, 1.f), FVector2(-1.f, 1.f), FVector4(1.f, 1.f, 1.f, 1.f)));
	Lod.Indice = { 0, 1, 2 };
	SFrameRes.PPTriangle = GDynamicRHI->CreateGeometry(Lod);
}

void FFrameResourceManager::CreatePPTriangleRR()
{
	FVector2 WidthAndHeight = FVector2(static_cast<float>(GEngine->GetWidth()), static_cast<float>(GEngine->GetHeight()));
	static const FVector4 BloomTint1 = FVector4(0.3465f);
	static const FVector4 BloomTint2 = FVector4(0.138f);
	static const FVector4 BloomTint3 = FVector4(0.1176f);
	static const FVector4 BloomTint4 = FVector4(0.066f);
	static const FVector4 BloomTint5 = FVector4(0.066f);
	static const float BloomIntensity = 1.0f;
	static const FVector4 BloomTintAs[3] = { BloomTint4, BloomTint3 * BloomIntensity, BloomTint2 * BloomIntensity };
	static const FVector4 BloomTintBs[3] = { BloomTint5, FVector4(1.0f, 1.0f, 1.0f, 0.0f), FVector4(1.0f, 1.0f, 1.0f, 0.0f) };

	FVertexInputLayer VIL_PostProcess;
	VIL_PostProcess.Elements.push_back({ "POSITION", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 0, 0, 0 });
	VIL_PostProcess.Elements.push_back({ "NORMAL", 0, FFormat::FORMAT_R32G32B32_FLOAT, 0, 12, 0, 0 });
	VIL_PostProcess.Elements.push_back({ "TEXCOORD", 0, FFormat::FORMAT_R32G32_FLOAT, 0, 24, 0, 0 });
	VIL_PostProcess.Elements.push_back({ "COLOR", 0, FFormat::FORMAT_R32G32B32A32_FLOAT, 0, 32, 0, 0 });

	FShaderInputLayer SIL_BloomSetup;
	SIL_BloomSetup.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	SIL_BloomSetup.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_BloomSetup.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });

	FShaderInputLayer SIL_BloomDown;
	SIL_BloomDown.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	SIL_BloomDown.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_BloomDown.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });

	FShaderInputLayer SIL_BloomUp;
	SIL_BloomUp.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	SIL_BloomUp.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_BloomUp.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_BloomUp.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_BloomUp.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });

	FShaderInputLayer SIL_SunMerge;
	SIL_SunMerge.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_ALL });
	SIL_SunMerge.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_SunMerge.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_SunMerge.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_SunMerge.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });

	FShaderInputLayer SIL_ToneMapping;
	SIL_ToneMapping.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_ToneMapping.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, FShaderVisibility::SHADER_VISIBILITY_PIXEL });
	SIL_ToneMapping.Elements.push_back({ FRangeType::DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, FShaderVisibility::SHADER_VISIBILITY_PIXEL });

	// bloom set up rr
	SFrameRes.RR_BloomSetup = CreateRenderResource
	(
		L"Resource\\BloomSetup.hlsl",
		256,
		VIL_PostProcess,
		SIL_BloomSetup,
		FFormat::FORMAT_R11G11B10_FLOAT,
		1,
		GDynamicRHI->GetFrameCount()
	);
	struct FBloomSetupCB
	{
		FVector4 BufferSizeAndInvSize;
		float BloomThreshold;
	} BloomSetupStruct;
	BloomSetupStruct.BufferSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / 4.f);
	BloomSetupStruct.BloomThreshold = 1.0f;
	for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
	{
		GDynamicRHI->WriteConstantBuffer(SFrameRes.RR_BloomSetup->CBs[i].get(), reinterpret_cast<void*>(&BloomSetupStruct), sizeof(FBloomSetupCB));
	}

	// bloom down rr
	for (uint32 i = 0; i < 4; i++)
	{
		SFrameRes.RR_BloomDown[i] = CreateRenderResource
		(
			L"Resource\\BloomDown.hlsl",
			256,
			VIL_PostProcess,
			SIL_BloomDown,
			FFormat::FORMAT_R11G11B10_FLOAT,
			1,
			GDynamicRHI->GetFrameCount()
		);
		struct FBloomDownCB
		{
			FVector4 BufferSizeAndInvSize;
			float BloomDownScale;
		} BloomDwonStruct;
		BloomDwonStruct.BufferSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / (static_cast<float>(pow(2, i + 3))));
		BloomDwonStruct.BloomDownScale = 0.66f * 4.0f;
		for (uint32 j = 0; j < GDynamicRHI->GetFrameCount(); j++)
		{
			GDynamicRHI->WriteConstantBuffer(SFrameRes.RR_BloomDown[i]->CBs[j].get(), reinterpret_cast<void*>(&BloomDwonStruct), sizeof(BloomDwonStruct));
		}
	}

	// bloom up rr
	for (int i = 0; i < 3; i++)
	{
		SFrameRes.RR_BloomUp[i] = CreateRenderResource
		(
			L"Resource\\BloomUp.hlsl",
			256,
			VIL_PostProcess,
			SIL_BloomUp,
			FFormat::FORMAT_R11G11B10_FLOAT,
			1,
			GDynamicRHI->GetFrameCount()
		);
		struct FBloomUpCB
		{
			FVector4 BufferASizeAndInvSize;
			FVector4 BufferBSizeAndInvSize;
			FVector4 BloomTintA;
			FVector4 BloomTintB;
			FVector2 BloomUpScales;
		} BloomUpStruct;
		BloomUpStruct.BufferASizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / static_cast<float>(pow(2, 5 - i))); // 32 16 8
		BloomUpStruct.BufferBSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / static_cast<float>(pow(2, 6 - i)));
		BloomUpStruct.BloomTintA = BloomTintAs[i] * (1.0f / 8.0f);
		BloomUpStruct.BloomTintB = BloomTintBs[i] * (1.0f / 8.0f);
		BloomUpStruct.BloomUpScales.x = 0.66f * 2.0f;
		BloomUpStruct.BloomUpScales.y = 0.66f * 2.0f;
		for (uint32 j = 0; j < GDynamicRHI->GetFrameCount(); j++)
		{
			GDynamicRHI->WriteConstantBuffer(SFrameRes.RR_BloomUp[i]->CBs[j].get(), reinterpret_cast<void*>(&BloomUpStruct), sizeof(BloomUpStruct));
		}
	}

	// sun merge
	SFrameRes.RR_SunMerge = CreateRenderResource
	(
		L"Resource\\SunMerge.hlsl",
		256,
		VIL_PostProcess,
		SIL_SunMerge,
		FFormat::FORMAT_R11G11B10_FLOAT,
		1,
		GDynamicRHI->GetFrameCount()
	);
	struct FSunMergeCB
	{
		FVector4 BloomUpSizeAndInvSize;
		FVector BloomColor;
	} SunMergeStruct;
	SunMergeStruct.BloomUpSizeAndInvSize = GetBufferSizeAndInvSize(WidthAndHeight / 8.f);
	SunMergeStruct.BloomColor = FVector(BloomTint1) * BloomIntensity * 0.5f;
	for (uint32 i = 0; i < GDynamicRHI->GetFrameCount(); i++)
	{
		GDynamicRHI->WriteConstantBuffer(SFrameRes.RR_SunMerge->CBs[i].get(), reinterpret_cast<void*>(&SunMergeStruct), sizeof(SunMergeStruct));
	}

	// tone mapping
	SFrameRes.RR_ToneMapping = CreateRenderResource
	(
		L"Resource\\ToneMapping.hlsl",
		256,
		VIL_PostProcess,
		SIL_ToneMapping,
		FFormat::FORMAT_R8G8B8A8_UNORM,
		1,
		GDynamicRHI->GetFrameCount()
	); // tone mapping need no constant buffer
}

void FFrameResourceManager::UpdateFrameResources(FScene* Scene, const uint32& FrameIndex)
{
	ACharacter* CurrentCharacter = Scene->GetCurrentCharacter();
	ACamera* CurrentCamera = Scene->GetCurrentCamera();
	
	// animation
	if (true)
	{
		struct PaletteCB
		{
			array<FMatrix, 68> GBoneTransforms;
		} CBInstance;

		for (uint32 i = 0; i < 68; i++)
		{
			CBInstance.GBoneTransforms[i] = glm::transpose(CurrentCharacter->GetSkeletalMeshCom()->GetAnimator().GetPalette_RenderThread()[i]);
		}
		GDynamicRHI->WriteConstantBuffer(MFrameRes[FrameIndex].CharacterPaletteCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CBInstance));
	}

	// camera
	if (CurrentCamera->IsDirty())
	{
		// camera vp
		const FMatrix& CV = CurrentCamera->GetViewMatrix_RenderThread();
		const FMatrix& CP = CurrentCamera->GetPerspProjMatrix_RenderThread();
		const FMatrix CamVP = glm::transpose(CP * CV);
		const FVector& CamPos = CurrentCamera->GetTransform().Translation;
		FVector4 Eye( CamPos.x, CamPos.y, CamPos.z, 1.f );
		struct CameraConstantBuffer
		{
			FMatrix CamVP;
			FVector4 Eye;
		} CBInstance = { CamVP, Eye };

		GDynamicRHI->WriteConstantBuffer(MFrameRes[FrameIndex].CameraCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CameraConstantBuffer));
		CurrentCamera->SetDirty(false);

		// direction light source cb should change when camera change
		FMatrix WVP = transpose(CP * CV * Scene->GetDirectionalLight()->GetStaticMeshComponent()->GetWorldMatrix());
		GDynamicRHI->WriteConstantBuffer(SFrameRes.RRMap_ScenePass[SFrameRes.DirectionalLight.get()]->CBs[FrameIndex].get(), reinterpret_cast<void*>(&WVP), sizeof(WVP));

		// point light source cb should change when camera change
		for (uint32 i = 0; i < Scene->GetPointLights().size(); i++)
		{
			FMatrix WVP = transpose(CP * CV * Scene->GetPointLights()[i]->GetStaticMeshComponent()->GetWorldMatrix());
			GDynamicRHI->WriteConstantBuffer(SFrameRes.RRMap_ScenePass[SFrameRes.PointLights[i].get()]->CBs[FrameIndex].get(), reinterpret_cast<void*>(&WVP), sizeof(WVP));
		}
	}

	// character position
	if (CurrentCharacter->IsPosDirty())
	{
		const FMatrix& V = Scene->GetDirectionalLight()->GetViewMatrix_RenderThread();
		const FMatrix& O = Scene->GetDirectionalLight()->GetOMatrix_RenderThread();
		FMatrix WVO = transpose(O * V * CurrentCharacter->GetSkeletalMeshCom()->GetWorldMatrix());
		FMatrix W = transpose(CurrentCharacter->GetSkeletalMeshCom()->GetWorldMatrix());
		GDynamicRHI->WriteConstantBuffer(SFrameRes.RRMap_ShadowPass[SFrameRes.CharacterMesh.get()]->CBs[FrameIndex].get(), reinterpret_cast<void*>(&WVO), sizeof(WVO));
		GDynamicRHI->WriteConstantBuffer(SFrameRes.RRMap_ScenePass[SFrameRes.CharacterMesh.get()]->CBs[FrameIndex].get(), reinterpret_cast<void*>(&W), sizeof(W));
		CurrentCharacter->SetPosDirty(false);
	}

	// static mesh
	for (uint32 i = 0; i < Scene->GetStaticMeshActors().size(); i++)
	{
		auto& Actor = Scene->GetStaticMeshActors()[i];
		if (Actor->IsDirty())
		{
			const FMatrix& V = Scene->GetDirectionalLight()->GetViewMatrix_RenderThread();
			const FMatrix& O = Scene->GetDirectionalLight()->GetOMatrix_RenderThread();
			FMatrix WVO = transpose(O * V * Actor->GetStaticMeshComponent()->GetWorldMatrix());
			FMatrix W = transpose(Actor->GetStaticMeshComponent()->GetWorldMatrix());
			GDynamicRHI->WriteConstantBuffer(SFrameRes.RRMap_ShadowPass[SFrameRes.StaticMeshes[i].get()]->CBs[FrameIndex].get(), reinterpret_cast<void*>(&WVO), sizeof(FMatrix));
			GDynamicRHI->WriteConstantBuffer(SFrameRes.RRMap_ScenePass[SFrameRes.StaticMeshes[i].get()]->CBs[FrameIndex].get(), reinterpret_cast<void*>(&W), sizeof(FMatrix));
			Actor->SetDirty(false);
		}
	}

	// directional light
	if (Scene->GetDirectionalLight()->IsDirty())
	{
		const FMatrix& V = Scene->GetDirectionalLight()->GetViewMatrix_RenderThread();
		const FMatrix& O = Scene->GetDirectionalLight()->GetOMatrix_RenderThread();
		struct LightCB
		{
			FMatrix VOMatrix;
			FMatrix ScreenMatrix;
			struct LightState
			{
				FVector DirectionalLightColor;
				float DirectionalLightIntensity;
				FVector DirectionalLightDir;
			} Light;
		} CBInstance;
		CBInstance.VOMatrix = glm::transpose(O * V);
		CBInstance.ScreenMatrix = glm::transpose(ProjToScreen);
		CBInstance.Light.DirectionalLightColor = Scene->GetDirectionalLight()->GetColor();
		CBInstance.Light.DirectionalLightIntensity = Scene->GetDirectionalLight()->GetIntensity();
		CBInstance.Light.DirectionalLightDir = glm::normalize(Scene->GetDirectionalLight()->GetDirection());

		GDynamicRHI->WriteConstantBuffer(SFrameRes.StaticDirectionalLightCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CBInstance));
		Scene->GetDirectionalLight()->SetDirty(false);
	}

	// point light
	for (uint32 i = 0; i < Scene->GetPointLights().size(); i++)
	{
		auto& Actor = Scene->GetPointLights()[i];
		if (Actor->IsDirty())
		{
			// point light is static now
		}
	}

}

//void FFrameResourceManager::UpdateFrameResCamera(FMatrix VP, FVector Eye, const uint32& FrameIndex)
//{
//	struct CameraConstantBuffer
//	{
//		FMatrix CamVP;
//		FVector Eye;
//	} CBInstance = { VP, Eye };
//	GDynamicRHI->WriteConstantBuffer(MFrameRes[FrameIndex].CameraCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CBInstance));
//}
//
//void FFrameResourceManager::UpdateFrameResPalette(vector<FMatrix> Palette, const uint32& FrameIndex)
//{
//	struct PaletteCB
//	{
//		array<FMatrix, 68> GBoneTransforms;
//	} CBInstance;
//
//	for (uint32 i = 0; i < 68; i++)
//	{
//		CBInstance.GBoneTransforms[i] = glm::transpose(Palette[i]);
//	}
//	GDynamicRHI->WriteConstantBuffer(MFrameRes[FrameIndex].CharacterPaletteCB.get(), reinterpret_cast<void*>(&CBInstance), sizeof(CBInstance));
//}