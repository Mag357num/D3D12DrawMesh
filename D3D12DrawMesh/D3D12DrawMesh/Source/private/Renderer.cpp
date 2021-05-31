#include "Renderer.h"
#include "Engine.h"

void FRenderer::Render(FDynamicRHI* RHI, FFrameResourceManager* FrameManager)
{
	RHI->FrameBegin();
	
	if (GEngine->UseShadow())
	{
		RenderShadow(RHI, FrameManager);
	}

	RenderScene(RHI, FrameManager);

	if (GEngine->UsePostProcess())
	{
		RenderPostProcess(RHI, FrameManager);
	}

	// transition
	RHI->FrameEnd();
}

void FRenderer::RenderShadow(FDynamicRHI* RHI, FFrameResourceManager* FrameManager)
{
	const uint32 FrameIndex = RHI->GetCurrentFramIndex();
	FMultiBufferFrameResource& MFrameRes = FrameManager->GetMultiFrameRes()[FrameIndex];
	FSingleBufferFrameResource& SFrameRes = FrameManager->GetSingleFrameRes();

	// shadow pass
	{
		SCOPED_EVENT("ShadowDepth");
		RHI->SetTextureState(MFrameRes.ShadowMap.get(), FRESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE);
		RHI->ClearDepthStencil(MFrameRes.ShadowMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(MFrameRes.ShadowMapSize), static_cast<float>(MFrameRes.ShadowMapSize), 0.f, 1.f);
		RHI->SetScissor(0, 0, MFrameRes.ShadowMapSize, MFrameRes.ShadowMapSize);
		RHI->SetRenderTarget(0, nullptr, MFrameRes.ShadowMap->DsvHandle.get());

		// draw character
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ShadowPass[SFrameRes.CharacterMesh.get()].get());

			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ShadowPass[SFrameRes.CharacterMesh.get()]->CBs[FrameIndex]->CBHandle);
			Handles.push_back(MFrameRes.CharacterPaletteCB->CBHandle);
			RHI->SetShaderInput(Handles);

			RHI->DrawMesh(SFrameRes.CharacterMesh.get());
		}

		// draw static mesh
		for (auto i : SFrameRes.StaticMeshes)
		{
			// use shadow pso
			RHI->SetPipelineState(SFrameRes.RRMap_ShadowPass[i.get()].get());

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ShadowPass[i.get()]->CBs[FrameIndex]->CBHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.get());
		}

		RHI->SetTextureState(MFrameRes.ShadowMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResourceManager* FrameManager)
{
	const uint32 FrameIndex = RHI->GetCurrentFramIndex();
	FMultiBufferFrameResource& MFrameRes = FrameManager->GetMultiFrameRes()[FrameIndex];
	FSingleBufferFrameResource& SFrameRes = FrameManager->GetSingleFrameRes();
	FHandle* Rt = GEngine->UsePostProcess() ? MFrameRes.SceneColorMap->RtvHandle.get() : RHI->GetBackBufferHandle();

	// scene color pass
	{
		SCOPED_EVENT("SceneColor");
		RHI->SetTextureState(MFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
		RHI->ClearRenderTarget(MFrameRes.SceneColorMap->RtvHandle.get());
		RHI->SetRenderTarget(1, Rt, MFrameRes.DepthStencilMap->DsvHandle.get());
		RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
		RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());

		// draw character
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.CharacterMesh.get()].get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ScenePass[SFrameRes.CharacterMesh.get()]->CBs[FrameIndex]->CBHandle);
			Handles.push_back(MFrameRes.CameraCB->CBHandle);
			Handles.push_back(SFrameRes.StaticSkyLightCB->CBHandle);
			Handles.push_back(MFrameRes.CharacterPaletteCB->CBHandle);
			Handles.push_back(MFrameRes.ShadowMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawMesh(SFrameRes.CharacterMesh.get());
		}

		// draw static mesh
		for (auto i : SFrameRes.StaticMeshes)
		{
			// pso
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[i.get()].get());

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ScenePass[i.get()]->CBs[FrameIndex]->CBHandle);
			Handles.push_back(MFrameRes.CameraCB->CBHandle);
			Handles.push_back(SFrameRes.StaticSkyLightCB->CBHandle);
			Handles.push_back(MFrameRes.ShadowMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.get());
		}
		RHI->SetTextureState(MFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void FRenderer::RenderPostProcess(FDynamicRHI* RHI, FFrameResourceManager* FrameManager)
{
	const uint32 FrameIndex = RHI->GetCurrentFramIndex();
	FMultiBufferFrameResource& MFrameRes = FrameManager->GetMultiFrameRes()[FrameIndex];
	FSingleBufferFrameResource& SFrameRes = FrameManager->GetSingleFrameRes();

	SCOPED_EVENT("Post Process");
	{
		auto& Tri = FrameManager->GetSingleFrameRes().PPTriangle;
		SCOPED_EVENT("Bloom");
		{
			// bloom setup
			{
				SCOPED_EVENT("Bloom setup");
				RHI->SetTextureState(MFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
				RHI->ClearRenderTarget(MFrameRes.BloomSetupMap->RtvHandle.get());
				RHI->SetRenderTarget(1, MFrameRes.BloomSetupMap->RtvHandle.get(), MFrameRes.DepthStencilMap->DsvHandle.get());
				RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
				RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
				RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);

				RHI->SetPipelineState(SFrameRes.RR_BloomSetup.get());

				vector<shared_ptr<FHandle>> Handles;
				Handles.push_back(SFrameRes.RR_BloomSetup->CBs[FrameIndex]->CBHandle);
				Handles.push_back(MFrameRes.SceneColorMap->SrvHandle);
				Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
				RHI->SetShaderInput(Handles);

				RHI->DrawMesh(Tri.get());
				RHI->SetTextureState(MFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}

			// bloom down
			{
				vector<shared_ptr<FHandle>> TexHandles;
				TexHandles.push_back(MFrameRes.BloomSetupMap->SrvHandle);
				TexHandles.push_back(MFrameRes.BloomDownMapArray[0]->SrvHandle);
				TexHandles.push_back(MFrameRes.BloomDownMapArray[1]->SrvHandle);
				TexHandles.push_back(MFrameRes.BloomDownMapArray[2]->SrvHandle);

				for (uint32 i = 0; i < 4; i++)
				{
					SCOPED_EVENT("Bloom down");
					RHI->SetTextureState(MFrameRes.BloomDownMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
					RHI->ClearRenderTarget(MFrameRes.BloomDownMapArray[i]->RtvHandle.get());
					RHI->SetRenderTarget(1, MFrameRes.BloomDownMapArray[i]->RtvHandle.get(), MFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / static_cast<uint32>(pow(2, 3 + i))), static_cast<float>(RHI->GetHeight() / static_cast<uint32>(pow(2, 3 + i))), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / static_cast<uint32>(pow(2, 3 + i)), RHI->GetHeight() / static_cast<uint32>(pow(2, 3 + i)));

					RHI->SetPipelineState(SFrameRes.RR_BloomDown[i].get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(SFrameRes.RR_BloomDown[i]->CBs[FrameIndex]->CBHandle);
					Handles.push_back(TexHandles[i]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->SetTextureState(MFrameRes.BloomDownMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			// bloom up
			{
				vector<shared_ptr<FHandle>> TexHandles1;
				TexHandles1.push_back(MFrameRes.BloomDownMapArray[2]->SrvHandle);
				TexHandles1.push_back(MFrameRes.BloomDownMapArray[1]->SrvHandle);
				TexHandles1.push_back(MFrameRes.BloomDownMapArray[0]->SrvHandle);

				vector<shared_ptr<FHandle>> TexHandles2;
				TexHandles2.push_back(MFrameRes.BloomDownMapArray[3]->SrvHandle);
				TexHandles2.push_back(MFrameRes.BloomUpMapArray[0]->SrvHandle);
				TexHandles2.push_back(MFrameRes.BloomUpMapArray[1]->SrvHandle);

				for (uint32 i = 0; i < 3; i++)
				{
					SCOPED_EVENT("Bloom up");
					RHI->SetTextureState(MFrameRes.BloomUpMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
					RHI->ClearRenderTarget(MFrameRes.BloomUpMapArray[i]->RtvHandle.get());
					RHI->SetRenderTarget(1, MFrameRes.BloomUpMapArray[i]->RtvHandle.get(), MFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / static_cast<uint32>(pow(2, 5 - i))), static_cast<float>(RHI->GetHeight() / static_cast<uint32>(pow(2, 5 - i))), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / static_cast<uint32>(pow(2, 5 - i)), RHI->GetHeight() / static_cast<uint32>(pow(2, 5 - i)));
					RHI->SetPipelineState(SFrameRes.RR_BloomUp[i].get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(SFrameRes.RR_BloomUp[i]->CBs[FrameIndex]->CBHandle);
					Handles.push_back(TexHandles1[i]);
					Handles.push_back(TexHandles2[i]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.WarpSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->SetTextureState(MFrameRes.BloomUpMapArray[i].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			// sun merge
			{
				SCOPED_EVENT("Sun Merge");
				RHI->SetTextureState(MFrameRes.SunMergeMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
				RHI->ClearRenderTarget(MFrameRes.SunMergeMap->RtvHandle.get());
				RHI->SetRenderTarget(1, MFrameRes.SunMergeMap->RtvHandle.get(), MFrameRes.DepthStencilMap->DsvHandle.get());
				RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
				RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
				RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);

				RHI->SetPipelineState(SFrameRes.RR_SunMerge.get());

				vector<shared_ptr<FHandle>> Handles;
				Handles.push_back(SFrameRes.RR_SunMerge->CBs[FrameIndex]->CBHandle);
				Handles.push_back(MFrameRes.BloomSetupMap->SrvHandle);
				Handles.push_back(MFrameRes.BloomUpMapArray[2]->SrvHandle);
				Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
				Handles.push_back(SFrameRes.WarpSampler->SamplerHandle);
				RHI->SetShaderInput(Handles);

				RHI->DrawMesh(Tri.get());
				RHI->SetTextureState(MFrameRes.SunMergeMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
		}

		// tonemapping output
		{
			SCOPED_EVENT("Tonemapping");
			RHI->ClearRenderTarget(RHI->GetBackBufferHandle());
			RHI->SetRenderTarget(1, RHI->GetBackBufferHandle(), MFrameRes.DepthStencilMap->DsvHandle.get());
			RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
			RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
			RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());

			RHI->SetPipelineState(SFrameRes.RR_ToneMapping.get());

			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(MFrameRes.SceneColorMap->SrvHandle);
			Handles.push_back(MFrameRes.SunMergeMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);

			RHI->DrawMesh(Tri.get());
		}
	}
}