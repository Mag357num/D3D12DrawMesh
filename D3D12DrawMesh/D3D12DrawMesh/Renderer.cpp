#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes)
{
	RHI->FrameBegin();

	// shadow pass
	{
		SCOPED_EVENT("ShadowDepth");
		RHI->ClearDepthStencil(FrameRes->GetShadowMap().get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(FrameRes->GetShadowMapSize()), static_cast<float>(FrameRes->GetShadowMapSize()), 0.f, 1.f);
		RHI->SetScissor(0, 0, FrameRes->GetShadowMapSize(), FrameRes->GetShadowMapSize());
		RHI->SetRenderTarget(0, nullptr, FrameRes->GetShadowMap()->DsvHandle.get());
		for (auto i : FrameRes->GetFrameMeshes())
		{
			// use shadow pso
			RHI->SetPipelineState(i.MeshRes->ShadowPipeline.get());

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(i.MeshRes->ShadowMat->CB->CBHandle);
			Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.Mesh.get());
		}
		RHI->TransitTextureState(FrameRes->GetShadowMap().get(), FRESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	// scene color pass
	{
		SCOPED_EVENT("SceneColor");
		RHI->ClearRenderTarget(FrameRes->GetSceneColorMap()->RtvHandle.get());
		RHI->SetRenderTarget(1, FrameRes->GetSceneColorMap()->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
		RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
		RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
		for (auto i : FrameRes->GetFrameMeshes())
		{
			// pso
			RHI->SetPipelineState(i.MeshRes->SceneColorPipeline.get()); // use HDR pso

			// root signature
			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(i.MeshRes->SceneColorMat->CB->CBHandle);
			Handles.push_back(FrameRes->GetShadowMap()->SrvHandle);
			Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
			RHI->SetShaderInput(Handles);

			// set mesh
			RHI->DrawMesh(i.Mesh.get());
		}
		RHI->TransitTextureState(FrameRes->GetSceneColorMap().get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	{
		SCOPED_EVENT("Post Process");
		{
			auto& TriRes = FrameRes->GetPostProcessTriangleRes();
			auto& Tri = FrameRes->GetPostProcessTriangle();

			SCOPED_EVENT("Bloom");
			{
				// bloom setup
				{
					SCOPED_EVENT("Bloom setup");
					RHI->ClearRenderTarget(FrameRes->GetBloomSetupMap()->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomSetupMap()->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
					RHI->SetPipelineState(TriRes->BloomSetupPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomSetupMat->CB->CBHandle);
					Handles.push_back(TriRes->BloomSetupMat->TexHandles[0]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomSetupMap().get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/4->1/8
				{
					SCOPED_EVENT("Bloom down 1/4->1/8");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[0]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[0]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
					RHI->SetPipelineState(TriRes->BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomDownMat[0]->CB->CBHandle);
					Handles.push_back(TriRes->BloomDownMat[0]->TexHandles[0]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[0].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/8->1/16
				{
					SCOPED_EVENT("Bloom down 1/8->1/16");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[1]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[1]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
					RHI->SetPipelineState(TriRes->BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomDownMat[1]->CB->CBHandle);
					Handles.push_back(TriRes->BloomDownMat[1]->TexHandles[0]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[1].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/16->1/32
				{
					SCOPED_EVENT("Bloom down 1/16->1/32");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[2]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[2]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
					RHI->SetPipelineState(TriRes->BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomDownMat[2]->CB->CBHandle);
					Handles.push_back(TriRes->BloomDownMat[2]->TexHandles[0]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[2].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/32->1/64
				{
					SCOPED_EVENT("Bloom down 1/32->1/64");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[3]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[3]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 64), static_cast<float>(RHI->GetHeight() / 64), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 64, RHI->GetHeight() / 64);
					RHI->SetPipelineState(TriRes->BloomDownPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomDownMat[3]->CB->CBHandle);
					Handles.push_back(TriRes->BloomDownMat[3]->TexHandles[0]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[3].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/64->1/32
				{
					SCOPED_EVENT("Bloom up 1/64->1/32");
					RHI->ClearRenderTarget(FrameRes->GetBloomUpMaps()[0]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomUpMaps()[0]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
					RHI->SetPipelineState(TriRes->BloomUpPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomUpMat[0]->CB->CBHandle);
					Handles.push_back(TriRes->BloomUpMat[0]->TexHandles[0]);
					Handles.push_back(TriRes->BloomUpMat[0]->TexHandles[1]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					Handles.push_back(FrameRes->GetWarpSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[0].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/32->1/16
				{
					SCOPED_EVENT("Bloom up 1/32->1/16");
					RHI->ClearRenderTarget(FrameRes->GetBloomUpMaps()[1]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomUpMaps()[1]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
					RHI->SetPipelineState(TriRes->BloomUpPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomUpMat[1]->CB->CBHandle);
					Handles.push_back(TriRes->BloomUpMat[1]->TexHandles[0]);
					Handles.push_back(TriRes->BloomUpMat[1]->TexHandles[1]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					Handles.push_back(FrameRes->GetWarpSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[1].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/16->1/8
				{
					SCOPED_EVENT("Bloom up 1/16->1/8");
					RHI->ClearRenderTarget(FrameRes->GetBloomUpMaps()[2]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomUpMaps()[2]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
					RHI->SetPipelineState(TriRes->BloomUpPipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->BloomUpMat[2]->CB->CBHandle);
					Handles.push_back(TriRes->BloomUpMat[2]->TexHandles[0]);
					Handles.push_back(TriRes->BloomUpMat[2]->TexHandles[1]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					Handles.push_back(FrameRes->GetWarpSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[2].get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// sun merge
				{
					SCOPED_EVENT("Sun Merge");
					RHI->ClearRenderTarget(FrameRes->GetSunMergeMap()->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetSunMergeMap()->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
					RHI->SetPipelineState(TriRes->SunMergePipeline.get());

					vector<shared_ptr<FHandle>> Handles;
					Handles.push_back(TriRes->SunMergeMat->CB->CBHandle);
					Handles.push_back(TriRes->SunMergeMat->TexHandles[0]);
					Handles.push_back(TriRes->SunMergeMat->TexHandles[1]);
					Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
					Handles.push_back(FrameRes->GetWarpSampler()->SamplerHandle);
					RHI->SetShaderInput(Handles);

					RHI->DrawMesh(Tri.get());
					RHI->TransitTextureState(FrameRes->GetSunMergeMap().get(), FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}

			// tonemapping output
			{
			SCOPED_EVENT("Tonemapping");
			RHI->ClearRenderTarget(RHI->GetBackBufferHandle());
			RHI->SetRenderTarget(1, RHI->GetBackBufferHandle(), FrameRes->GetDsMap()->DsvHandle.get());
			RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
			RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
			RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
			RHI->SetPipelineState(TriRes->ToneMappingPipeline.get());

			vector<shared_ptr<FHandle>> Handles;
			Handles.push_back(TriRes->ToneMappingMat->TexHandles[0]);
			Handles.push_back(TriRes->ToneMappingMat->TexHandles[1]);
			Handles.push_back(FrameRes->GetClampSampler()->SamplerHandle);
			RHI->SetShaderInput(Handles);

			RHI->DrawMesh(Tri.get());
			}
		}
	}

	// transition
	RHI->TransitTextureState(FrameRes->GetShadowMap().get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE);
	RHI->TransitTextureState(FrameRes->GetSceneColorMap().get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomSetupMap().get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[0].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[1].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[2].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[3].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[0].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[1].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[2].get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetSunMergeMap().get(), FRESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET);

	RHI->FrameEnd();
}
