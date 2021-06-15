#include "Renderer.h"
#include "Engine.h"

void FRenderer::Render(FDynamicRHI* RHI, const uint32& FrameIndex, FFrameResourceManager* FRManager)
{
	FMultiBufferFrameResource& MFrameRes = FRManager->GetMultiFrameRes()[FrameIndex];
	FSingleBufferFrameResource& SFrameRes = FRManager->GetSingleFrameRes();

	RHI->FrameBegin();
	
	if (GEngine->UseShadow())
	{
		RenderShadow(RHI, FrameIndex, SFrameRes, MFrameRes);
	}

	RenderScene(RHI, FrameIndex, SFrameRes, MFrameRes);

	if (GEngine->UsePostProcess())
	{
		RenderPostProcess(RHI, FrameIndex, SFrameRes, MFrameRes);
	}

	// transition
	RHI->FrameEnd();
}

void FRenderer::RenderShadow(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes)
{
	// shadow pass
	{
		SCOPED_EVENT("ShadowDepth");
		RHI->SetTextureState(SFrameRes.ShadowMap.get(), FRESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE);
		RHI->ClearDepthStencil(SFrameRes.ShadowMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(SFrameRes.ShadowMapSize), static_cast<float>(SFrameRes.ShadowMapSize), 0.f, 1.f);
		RHI->SetScissor(0, 0, SFrameRes.ShadowMapSize, SFrameRes.ShadowMapSize);
		RHI->SetRenderTarget(0, nullptr, SFrameRes.ShadowMap->DsvHandle.get());

		// draw character
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ShadowPass[SFrameRes.CharacterMesh.get()].get());

			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(MFrameRes.Character_ShadowPass_LocatingCB->CBHandle);
			Handles.push_back(MFrameRes.CharacterPaletteCB->CBHandle);
			RHI->SetShaderInput(Handles);

			RHI->DrawGeometry(SFrameRes.CharacterMesh.get());
		}

		// draw static mesh
		RHI->SetPipelineState(SFrameRes.RRMap_ShadowPass[SFrameRes.StaticMeshes[0].get()].get()); // for loop use same pso, set ahead avoid extra cost
		for (uint32 i = 0; i < SFrameRes.StaticMeshes.size(); i++)
		{
			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(MFrameRes.StaticMesh_ShadowPass_LocatingCBs[i]->CBHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawGeometry(SFrameRes.StaticMeshes[i].get());
		}

		RHI->SetTextureState(SFrameRes.ShadowMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void FRenderer::RenderScene(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes)
{
	FHandle* Rt = GEngine->UsePostProcess() ? SFrameRes.SceneColorMap->RtvHandle.get() : RHI->GetBackBufferHandle();
	const uint32& Width = GEngine->GetWidth();
	const uint32& Height = GEngine->GetHeight();

	// scene color pass
	{
		SCOPED_EVENT("SceneColor");
		RHI->SetTextureState(SFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
		RHI->ClearRenderTarget(SFrameRes.SceneColorMap->RtvHandle.get());
		RHI->SetRenderTarget(1, Rt, SFrameRes.DepthStencilMap->DsvHandle.get());
		RHI->ClearDepthStencil(SFrameRes.DepthStencilMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height), 0.f, 1.f);
		RHI->SetScissor(0, 0, Width, Height);

		// draw light source mesh
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.DirectionalLightMesh.get()].get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(MFrameRes.DirectionalLight_LocatingCB->CBHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawGeometry(SFrameRes.DirectionalLightMesh.get());
		}

		// draw light source mesh
		RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.PointLightMeshes[0].get()].get()); // for loop use same pso, set ahead avoid extra cost
		for (uint32 i = 0; i < SFrameRes.PointLightMeshes.size(); i++)
		{
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(MFrameRes.PointLight_LocatingCBs[i]->CBHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawGeometry(SFrameRes.PointLightMeshes[i].get());
		}

		// draw character
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.CharacterMesh.get()].get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(MFrameRes.Character_ScenePass_LocatingCB->CBHandle);
			Handles.push_back(MFrameRes.CameraCB->CBHandle);
			Handles.push_back(MFrameRes.DirectionalLight_LightingInfoCB->CBHandle);
			Handles.push_back(MFrameRes.PointLights_LightingInfoCB->CBHandle);
			Handles.push_back(MFrameRes.CharacterPaletteCB->CBHandle);
			Handles.push_back(SFrameRes.ShadowMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawGeometry(SFrameRes.CharacterMesh.get());
		}

		// draw static mesh
		for (uint32 i = 0; i < SFrameRes.StaticMeshes.size(); i++)
		{
			if (SFrameRes.RRMap_ScenePass[SFrameRes.StaticMeshes[i].get()]->BlendMode == FBlendMode::TRANSLUCENT_BM)
			{
				continue;
			}

			//pso
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.StaticMeshes[i].get()].get()); // for loop use same pso, set ahead avoid extra cost

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			Handles.push_back(MFrameRes.StaticMesh_ScenePass_LocatingCBs[i]->CBHandle);
			Handles.push_back(MFrameRes.CameraCB->CBHandle);
			Handles.push_back(MFrameRes.DirectionalLight_LightingInfoCB->CBHandle);
			Handles.push_back(MFrameRes.PointLights_LightingInfoCB->CBHandle);
			Handles.push_back(SFrameRes.MaterialCBs[SFrameRes.StaticMeshes[i].get()]->CBHandle);
			Handles.push_back(SFrameRes.ShadowMap->SrvHandle);
			for (auto j : SFrameRes.MaterialTexs[SFrameRes.StaticMeshes[i].get()])
			{
				Handles.push_back(j->SrvHandle);
			}
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawGeometry(SFrameRes.StaticMeshes[i].get());
		}

		auto Map = SFrameRes.TranslucentActorIndice;
		for (map<float, uint32>::reverse_iterator Iter = Map.rbegin(); Iter != Map.rend(); Iter++) // make sure the opaque actor render before translucent actor
		{
			uint32 i = Iter->second;

			//pso
			RHI->SetPipelineState( SFrameRes.RRMap_ScenePass[SFrameRes.StaticMeshes[i].get()].get() ); // for loop use same pso, set ahead avoid extra cost

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			Handles.push_back(MFrameRes.StaticMesh_ScenePass_LocatingCBs[i]->CBHandle);
			Handles.push_back( MFrameRes.CameraCB->CBHandle );
			Handles.push_back( MFrameRes.DirectionalLight_LightingInfoCB->CBHandle );
			Handles.push_back( MFrameRes.PointLights_LightingInfoCB->CBHandle );
			Handles.push_back( SFrameRes.MaterialCBs[SFrameRes.StaticMeshes[i].get()]->CBHandle );
			Handles.push_back(SFrameRes.ShadowMap->SrvHandle);
			for (auto j : SFrameRes.MaterialTexs[SFrameRes.StaticMeshes[i].get()])
			{
				Handles.push_back(j->SrvHandle);
			}
			RHI->SetShaderInput( Handles );

			// set mesh
			RHI->DrawGeometry( SFrameRes.StaticMeshes[i].get() );
		}
		RHI->SetTextureState(SFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void FRenderer::RenderPostProcess(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes)
{
	const uint32& Width = GEngine->GetWidth();
	const uint32& Height = GEngine->GetHeight();

	SCOPED_EVENT("Post Process");
	{
		auto& Tri = SFrameRes.PPTriangle;
		SCOPED_EVENT("Bloom");
		{
			// bloom setup
			{
				SCOPED_EVENT("Bloom setup");
				RHI->SetTextureState(SFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
				RHI->ClearRenderTarget(SFrameRes.BloomSetupMap->RtvHandle.get());
				RHI->SetRenderTarget(1, SFrameRes.BloomSetupMap->RtvHandle.get(), SFrameRes.DepthStencilMap->DsvHandle.get());
				RHI->ClearDepthStencil(SFrameRes.DepthStencilMap.get());
				RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / 4), static_cast<float>(Height / 4), 0.f, 1.f);
				RHI->SetScissor(0, 0, Width / 4, Height / 4);

				RHI->SetPipelineState(SFrameRes.RR_BloomSetup.get());

				vector<shared_ptr<FHandle>> Handles;
				Handles.push_back(SFrameRes.BloomSetupCB->CBHandle);
				Handles.push_back(SFrameRes.SceneColorMap->SrvHandle);
				Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
				RHI->SetShaderInput(Handles);

				RHI->DrawGeometry(Tri.get());
				RHI->SetTextureState(SFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}

			// bloom down
			{
				vector<shared_ptr<FHandle>> TexHandles;
				TexHandles.push_back(SFrameRes.BloomSetupMap->SrvHandle);
				TexHandles.push_back(SFrameRes.BloomDownMapArray[0]->SrvHandle);
				TexHandles.push_back(SFrameRes.BloomDownMapArray[1]->SrvHandle);
				TexHandles.push_back(SFrameRes.BloomDownMapArray[2]->SrvHandle);

				RHI->SetPipelineState(SFrameRes.RR_BloomDown[0].get()); // for loop use same pso, set ahead avoid extra cost
				for (uint32 i = 0; i < 4; i++)
				{
					SCOPED_EVENT("Bloom down");
					RHI->SetTextureState(SFrameRes.BloomDownMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
					RHI->ClearRenderTarget(SFrameRes.BloomDownMapArray[i]->RtvHandle.get());
					RHI->SetRenderTarget(1, SFrameRes.BloomDownMapArray[i]->RtvHandle.get(), SFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(SFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / static_cast<uint32>(pow(2, 3 + i))), static_cast<float>(Height / static_cast<uint32>(pow(2, 3 + i))), 0.f, 1.f);
					RHI->SetScissor(0, 0, Width / static_cast<uint32>(pow(2, 3 + i)), Height / static_cast<uint32>(pow(2, 3 + i)));

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(SFrameRes.BloomDownCBs[i]->CBHandle);
					Handles.push_back(TexHandles[i]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawGeometry(Tri.get());
					RHI->SetTextureState(SFrameRes.BloomDownMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			// bloom up
			{
				vector<shared_ptr<FHandle>> TexHandles1;
				TexHandles1.push_back(SFrameRes.BloomDownMapArray[2]->SrvHandle);
				TexHandles1.push_back(SFrameRes.BloomDownMapArray[1]->SrvHandle);
				TexHandles1.push_back(SFrameRes.BloomDownMapArray[0]->SrvHandle);

				vector<shared_ptr<FHandle>> TexHandles2;
				TexHandles2.push_back(SFrameRes.BloomDownMapArray[3]->SrvHandle);
				TexHandles2.push_back(SFrameRes.BloomUpMapArray[0]->SrvHandle);
				TexHandles2.push_back(SFrameRes.BloomUpMapArray[1]->SrvHandle);

				RHI->SetPipelineState(SFrameRes.RR_BloomUp[0].get()); // for loop use same pso, set ahead avoid extra cost
				for (uint32 i = 0; i < 3; i++)
				{
					SCOPED_EVENT("Bloom up");
					RHI->SetTextureState(SFrameRes.BloomUpMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
					RHI->ClearRenderTarget(SFrameRes.BloomUpMapArray[i]->RtvHandle.get());
					RHI->SetRenderTarget(1, SFrameRes.BloomUpMapArray[i]->RtvHandle.get(), SFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(SFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / static_cast<uint32>(pow(2, 5 - i))), static_cast<float>(Height / static_cast<uint32>(pow(2, 5 - i))), 0.f, 1.f);
					RHI->SetScissor(0, 0, Width / static_cast<uint32>(pow(2, 5 - i)), Height / static_cast<uint32>(pow(2, 5 - i)));

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(SFrameRes.BloomUpCBs[i]->CBHandle);
					Handles.push_back(TexHandles1[i]);
					Handles.push_back(TexHandles2[i]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawGeometry(Tri.get());
					RHI->SetTextureState(SFrameRes.BloomUpMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			// sun merge
			{
				SCOPED_EVENT("Sun Merge");
				RHI->SetTextureState(SFrameRes.SunMergeMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
				RHI->ClearRenderTarget(SFrameRes.SunMergeMap->RtvHandle.get());
				RHI->SetRenderTarget(1, SFrameRes.SunMergeMap->RtvHandle.get(), SFrameRes.DepthStencilMap->DsvHandle.get());
				RHI->ClearDepthStencil(SFrameRes.DepthStencilMap.get());
				RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / 4), static_cast<float>(Height / 4), 0.f, 1.f);
				RHI->SetScissor(0, 0, Width / 4, Height / 4);

				RHI->SetPipelineState(SFrameRes.RR_SunMerge.get());

				vector<shared_ptr<FHandle>> Handles;
				Handles.push_back(SFrameRes.SunMergeCB->CBHandle);
				Handles.push_back(SFrameRes.BloomSetupMap->SrvHandle);
				Handles.push_back(SFrameRes.BloomUpMapArray[2]->SrvHandle);
				Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
				Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
				RHI->SetShaderInput(Handles);

				RHI->DrawGeometry(Tri.get());
				RHI->SetTextureState(SFrameRes.SunMergeMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
		}

		// tonemapping output
		{
			SCOPED_EVENT("Tonemapping");
			RHI->ClearRenderTarget(RHI->GetBackBufferHandle());
			RHI->SetRenderTarget(1, RHI->GetBackBufferHandle(), SFrameRes.DepthStencilMap->DsvHandle.get());
			RHI->ClearDepthStencil(SFrameRes.DepthStencilMap.get());
			RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height), 0.f, 1.f);
			RHI->SetScissor(0, 0, Width, Height);

			RHI->SetPipelineState(SFrameRes.RR_ToneMapping.get());

			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.SceneColorMap->SrvHandle);
			Handles.push_back(SFrameRes.SunMergeMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);

			RHI->DrawGeometry(Tri.get());
		}
	}
}