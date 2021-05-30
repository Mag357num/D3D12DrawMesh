#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResourceManager* FrameManager)
{
	RHI->FrameBegin();
	FMultiBufferFrameResource& DFrameRes = FrameManager->GetDynamicFrameRes()[GDynamicRHI->GetCurrentFramIndex()];
	FSingleBufferFrameResource& SFrameRes = FrameManager->GetStaticFrameRes();
	
	// shadow pass
	{
		SCOPED_EVENT("ShadowDepth");
		RHI->ClearDepthStencil(DFrameRes.ShadowMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(DFrameRes.ShadowMapSize), static_cast<float>(DFrameRes.ShadowMapSize), 0.f, 1.f);
		RHI->SetScissor(0, 0, DFrameRes.ShadowMapSize, DFrameRes.ShadowMapSize);
		RHI->SetRenderTarget(0, nullptr, DFrameRes.ShadowMap->DsvHandle.get());
		// draw static mesh
		for (auto i : DFrameRes.StaticMeshArray)
		{
			// use shadow pso
			RHI->SetPipelineState(i.MeshRes->ShadowPipeline.get());

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(i.MeshRes->SceneColorMat->CB->CBHandle);
			Handles.push_back(SFrameRes.StaticSkyLightCB->CBHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.Mesh.get());
		}

		// draw skeletalmesh
		{
			RHI->SetPipelineState(DFrameRes.SkeletalMesh.MeshRes->ShadowPipeline.get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(DFrameRes.SkeletalMesh.MeshRes->SceneColorMat->CB->CBHandle);
			Handles.push_back(SFrameRes.StaticSkyLightCB->CBHandle);
			Handles.push_back(DFrameRes.CharacterPaletteCB->CBHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawMesh(DFrameRes.SkeletalMesh.Mesh.get());
		}

		RHI->TransitTextureState(DFrameRes.ShadowMap.get(), FRESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	// scene color pass
	{
		SCOPED_EVENT("SceneColor");
		RHI->ClearRenderTarget(DFrameRes.SceneColorMap->RtvHandle.get());
		RHI->SetRenderTarget(1, DFrameRes.SceneColorMap->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
		RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
		RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
		
		// draw static mesh
		for (auto i : DFrameRes.StaticMeshArray)
		{
			// pso
			RHI->SetPipelineState(i.MeshRes->SceneColorPipeline.get()); // use HDR pso

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(i.MeshRes->SceneColorMat->CB->CBHandle);
			Handles.push_back(DFrameRes.CameraCB->CBHandle);
			Handles.push_back(SFrameRes.StaticSkyLightCB->CBHandle);
			Handles.push_back(DFrameRes.ShadowMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.Mesh.get());
		}

		// draw skeletalmesh
		{
			RHI->SetPipelineState(DFrameRes.SkeletalMesh.MeshRes->SceneColorPipeline.get());
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(DFrameRes.SkeletalMesh.MeshRes->SceneColorMat->CB->CBHandle);
			Handles.push_back(DFrameRes.CameraCB->CBHandle);
			Handles.push_back(SFrameRes.StaticSkyLightCB->CBHandle);
			Handles.push_back(DFrameRes.CharacterPaletteCB->CBHandle);
			Handles.push_back(DFrameRes.ShadowMap->SrvHandle);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);
			RHI->DrawMesh(DFrameRes.SkeletalMesh.Mesh.get());
		}

		RHI->TransitTextureState(DFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	{
		SCOPED_EVENT("Post Process");
		{
			auto& Tri = FrameManager->GetStaticFrameRes().PostProcessTriangle;

			SCOPED_EVENT("Bloom");
			{
				// bloom setup
				{
					SCOPED_EVENT("Bloom setup");
					RHI->ClearRenderTarget(DFrameRes.BloomSetupMap->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomSetupMap->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
					RHI->SetPipelineState(DFrameRes.BloomSetupPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomSetupMat->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomSetupMat->TexHandles[0]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/4->1/8
				{
					SCOPED_EVENT("Bloom down 1/4->1/8");
					RHI->ClearRenderTarget(DFrameRes.BloomDownMapArray[0]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomDownMapArray[0]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
					RHI->SetPipelineState(DFrameRes.BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomDownMat[0]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomDownMat[0]->TexHandles[0]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomDownMapArray[0].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/8->1/16
				{
					SCOPED_EVENT("Bloom down 1/8->1/16");
					RHI->ClearRenderTarget(DFrameRes.BloomDownMapArray[1]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomDownMapArray[1]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
					RHI->SetPipelineState(DFrameRes.BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomDownMat[1]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomDownMat[1]->TexHandles[0]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomDownMapArray[1].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/16->1/32
				{
					SCOPED_EVENT("Bloom down 1/16->1/32");
					RHI->ClearRenderTarget(DFrameRes.BloomDownMapArray[2]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomDownMapArray[2]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
					RHI->SetPipelineState(DFrameRes.BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomDownMat[2]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomDownMat[2]->TexHandles[0]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomDownMapArray[2].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/32->1/64
				{
					SCOPED_EVENT("Bloom down 1/32->1/64");
					RHI->ClearRenderTarget(DFrameRes.BloomDownMapArray[3]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomDownMapArray[3]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 64), static_cast<float>(RHI->GetHeight() / 64), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 64, RHI->GetHeight() / 64);
					RHI->SetPipelineState(DFrameRes.BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomDownMat[3]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomDownMat[3]->TexHandles[0]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomDownMapArray[3].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/64->1/32
				{
					SCOPED_EVENT("Bloom up 1/64->1/32");
					RHI->ClearRenderTarget(DFrameRes.BloomUpMapArray[0]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomUpMapArray[0]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
					RHI->SetPipelineState(DFrameRes.BloomUpPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomUpMat[0]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomUpMat[0]->TexHandles[0]);
					Handles.push_back(DFrameRes.BloomUpMat[0]->TexHandles[1]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.WarpSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomUpMapArray[0].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/32->1/16
				{
					SCOPED_EVENT("Bloom up 1/32->1/16");
					RHI->ClearRenderTarget(DFrameRes.BloomUpMapArray[1]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomUpMapArray[1]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
					RHI->SetPipelineState(DFrameRes.BloomUpPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomUpMat[1]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomUpMat[1]->TexHandles[0]);
					Handles.push_back(DFrameRes.BloomUpMat[1]->TexHandles[1]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.WarpSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomUpMapArray[1].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/16->1/8
				{
					SCOPED_EVENT("Bloom up 1/16->1/8");
					RHI->ClearRenderTarget(DFrameRes.BloomUpMapArray[2]->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.BloomUpMapArray[2]->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
					RHI->SetPipelineState(DFrameRes.BloomUpPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.BloomUpMat[2]->CB->CBHandle);
					Handles.push_back(DFrameRes.BloomUpMat[2]->TexHandles[0]);
					Handles.push_back(DFrameRes.BloomUpMat[2]->TexHandles[1]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.WarpSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.BloomUpMapArray[2].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// sun merge
				{
					SCOPED_EVENT("Sun Merge");
					RHI->ClearRenderTarget(DFrameRes.SunMergeMap->RtvHandle.get());
					RHI->SetRenderTarget(1, DFrameRes.SunMergeMap->RtvHandle.get(), DFrameRes.DepthStencilMap->DsvHandle.get());
					RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
					RHI->SetPipelineState(DFrameRes.SunMergePipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(DFrameRes.SunMergeMat->CB->CBHandle);
					Handles.push_back(DFrameRes.SunMergeMat->TexHandles[0]);
					Handles.push_back(DFrameRes.SunMergeMat->TexHandles[1]);
					Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
					Handles.push_back(SFrameRes.WarpSampler->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(DFrameRes.SunMergeMap.get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			// tonemapping output
			{
			SCOPED_EVENT("Tonemapping");
			RHI->ClearRenderTarget(RHI->GetBackBufferHandle());
			RHI->SetRenderTarget(1, RHI->GetBackBufferHandle(), DFrameRes.DepthStencilMap->DsvHandle.get());
			RHI->ClearDepthStencil(DFrameRes.DepthStencilMap.get());
			RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
			RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
			RHI->SetPipelineState(DFrameRes.ToneMappingPipeline.get());

			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(DFrameRes.ToneMappingMat->TexHandles[0]);
			Handles.push_back(DFrameRes.ToneMappingMat->TexHandles[1]);
			Handles.push_back(SFrameRes.ClampSampler->SamplerHandle);
			RHI->SetShaderInput(Handles);

			RHI->DrawMesh(Tri.get());
			}
		}
	}

	// transition
	RHI->TransitTextureState(DFrameRes.ShadowMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE);
	RHI->TransitTextureState(DFrameRes.SceneColorMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomSetupMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomDownMapArray[0].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomDownMapArray[1].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomDownMapArray[2].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomDownMapArray[3].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomUpMapArray[0].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomUpMapArray[1].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.BloomUpMapArray[2].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(DFrameRes.SunMergeMap.get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);

	RHI->FrameEnd();
}
