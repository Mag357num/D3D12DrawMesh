#include "Renderer.h"
#include "Engine.h"

void FRenderer::Render(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes)
{
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

void FRenderer::RenderScene(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes)
{
	FHandle* Rt = GEngine->UsePostProcess() ? MFrameRes.SceneColorMap->RtvHandle.get() : RHI->GetBackBufferHandle();
	const uint32& Width = GEngine->GetWidth();
	const uint32& Height = GEngine->GetHeight();

	// scene color pass
	{
		SCOPED_EVENT("SceneColor");
		RHI->SetTextureState(MFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
		RHI->ClearRenderTarget(MFrameRes.SceneColorMap->RtvHandle.get());
		RHI->SetRenderTarget(1, Rt, MFrameRes.DepthStencilMap->DsvHandle.get());
		RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height), 0.f, 1.f);
		RHI->SetScissor(0, 0, Width, Height);

		// draw light source mesh
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.DirectionalLight.get()].get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ScenePass[SFrameRes.DirectionalLight.get()]->CBs[FrameIndex]->CBHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawMesh(SFrameRes.DirectionalLight.get());
		}

		// draw light source mesh
		for (auto i : SFrameRes.PointLights)
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[i.get()].get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ScenePass[i.get()]->CBs[FrameIndex]->CBHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawMesh(i.get());
		}

		// draw character
		{
			RHI->SetPipelineState(SFrameRes.RRMap_ScenePass[SFrameRes.CharacterMesh.get()].get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(SFrameRes.RRMap_ScenePass[SFrameRes.CharacterMesh.get()]->CBs[FrameIndex]->CBHandle);
			Handles.push_back(MFrameRes.CameraCB->CBHandle);
			Handles.push_back(SFrameRes.StaticDirectionalLightCB->CBHandle);
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
			Handles.push_back(SFrameRes.StaticDirectionalLightCB->CBHandle);
			Handles.push_back(MFrameRes.ShadowMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.get());
		}
		RHI->SetTextureState(MFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
				RHI->SetTextureState(MFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
				RHI->ClearRenderTarget(MFrameRes.BloomSetupMap->RtvHandle.get());
				RHI->SetRenderTarget(1, MFrameRes.BloomSetupMap->RtvHandle.get(), MFrameRes.DepthStencilMap->DsvHandle.get());
				RHI->ClearDepthStencil(MFrameRes.DepthStencilMap.get());
				RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / 4), static_cast<float>(Height / 4), 0.f, 1.f);
				RHI->SetScissor(0, 0, Width / 4, Height / 4);

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
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / static_cast<uint32>(pow(2, 3 + i))), static_cast<float>(Height / static_cast<uint32>(pow(2, 3 + i))), 0.f, 1.f);
					RHI->SetScissor(0, 0, Width / static_cast<uint32>(pow(2, 3 + i)), Height / static_cast<uint32>(pow(2, 3 + i)));

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
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / static_cast<uint32>(pow(2, 5 - i))), static_cast<float>(Height / static_cast<uint32>(pow(2, 5 - i))), 0.f, 1.f);
					RHI->SetScissor(0, 0, Width / static_cast<uint32>(pow(2, 5 - i)), Height / static_cast<uint32>(pow(2, 5 - i)));
					RHI->SetPipelineState(SFrameRes.RR_BloomUp[i].get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(SFrameRes.RR_BloomUp[i]->CBs[FrameIndex]->CBHandle);
					Handles.push_back(TexHandles1[i]);
					Handles.push_back(TexHandles2[i]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
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
				RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width / 4), static_cast<float>(Height / 4), 0.f, 1.f);
				RHI->SetScissor(0, 0, Width / 4, Height / 4);

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
			RHI->SetViewport(0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height), 0.f, 1.f);
			RHI->SetScissor(0, 0, Width, Height);

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