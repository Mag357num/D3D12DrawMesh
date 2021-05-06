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
			RHI->ChoosePipelineState(i.MeshRes->ShadowMat->PSO.get());

			// root signature
			RHI->SetShaderInput(FPassType::SHADOW_PT, i.MeshRes->ShadowMat.get(), FrameRes);

			// set mesh
			RHI->DrawMesh(i.Mesh.get());
		}
		RHI->TransitTextureState(FrameRes->GetShadowMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
			RHI->ChoosePipelineState(i.MeshRes->SceneColorMat->PSO.get()); // use HDR pso

			// root signature
			RHI->SetShaderInput(FPassType::SCENE_COLOR_PT, i.MeshRes->SceneColorMat.get(), FrameRes);

			// set mesh
			RHI->DrawMesh(i.Mesh.get());
		}
		RHI->TransitTextureState(FrameRes->GetSceneColorMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	{
		SCOPED_EVENT("Post Process");
		{
			SCOPED_EVENT("Bloom setup");
			{
				// bloom setup
				{
					SCOPED_EVENT("Bloom");
					RHI->ClearRenderTarget(FrameRes->GetBloomSetupMap()->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomSetupMap()->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomSetupMat->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_SETUP_PT, FrameRes->GetPostProcessTriangleRes()->BloomSetupMat.get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomSetupMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/4->1/8
				{
					SCOPED_EVENT("Bloom down 1/4->1/8");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[0]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[0]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomDownMat[0]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->GetPostProcessTriangleRes()->BloomDownMat[0].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[0].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/8->1/16
				{
					SCOPED_EVENT("Bloom down 1/8->1/16");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[1]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[1]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomDownMat[1]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->GetPostProcessTriangleRes()->BloomDownMat[1].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[1].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/16->1/32
				{
					SCOPED_EVENT("Bloom down 1/16->1/32");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[2]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[2]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomDownMat[2]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->GetPostProcessTriangleRes()->BloomDownMat[2].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[2].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom down 1/32->1/64
				{
					SCOPED_EVENT("Bloom down 1/32->1/64");
					RHI->ClearRenderTarget(FrameRes->GetBloomDownMaps()[3]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomDownMaps()[3]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 64), static_cast<float>(RHI->GetHeight() / 64), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 64, RHI->GetHeight() / 64);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomDownMat[3]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->GetPostProcessTriangleRes()->BloomDownMat[3].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[3].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/64->1/32
				{
					SCOPED_EVENT("Bloom up 1/64->1/32");
					RHI->ClearRenderTarget(FrameRes->GetBloomUpMaps()[0]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomUpMaps()[0]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomUpMat[0]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_UP_PT, FrameRes->GetPostProcessTriangleRes()->BloomUpMat[0].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[0].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/32->1/16
				{
					SCOPED_EVENT("Bloom up 1/32->1/16");
					RHI->ClearRenderTarget(FrameRes->GetBloomUpMaps()[1]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomUpMaps()[1]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomUpMat[1]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_UP_PT, FrameRes->GetPostProcessTriangleRes()->BloomUpMat[1].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[1].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// bloom up 1/16->1/8
				{
					SCOPED_EVENT("Bloom up 1/16->1/8");
					RHI->ClearRenderTarget(FrameRes->GetBloomUpMaps()[2]->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetBloomUpMaps()[2]->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->BloomUpMat[2]->PSO.get());
					RHI->SetShaderInput(FPassType::BLOOM_UP_PT, FrameRes->GetPostProcessTriangleRes()->BloomUpMat[2].get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[2].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}

				// sun merge
				{
					SCOPED_EVENT("Sun Merge");
					RHI->ClearRenderTarget(FrameRes->GetSunMergeMap()->RtvHandle.get());
					RHI->SetRenderTarget(1, FrameRes->GetSunMergeMap()->RtvHandle.get(), FrameRes->GetDsMap()->DsvHandle.get());
					RHI->ClearDepthStencil(FrameRes->GetDsMap().get());
					RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
					RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
					RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->SunMergeMat->PSO.get());
					RHI->SetShaderInput(FPassType::SUN_MERGE_PT, FrameRes->GetPostProcessTriangleRes()->SunMergeMat.get(), FrameRes);
					RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
					RHI->TransitTextureState(FrameRes->GetSunMergeMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
			}
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
		RHI->ChoosePipelineState(FrameRes->GetPostProcessTriangleRes()->ToneMappingMat->PSO.get());
		RHI->SetShaderInput(FPassType::TONEMAPPING_PT, FrameRes->GetPostProcessTriangleRes()->ToneMappingMat.get(), FrameRes);
		RHI->DrawMesh(FrameRes->GetPostProcessTriangle().get());
	}

	// transition
	RHI->TransitTextureState(FrameRes->GetShadowMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE);
	RHI->TransitTextureState(FrameRes->GetSceneColorMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomSetupMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[0].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[1].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[2].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomDownMaps()[3].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[0].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[1].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetBloomUpMaps()[2].get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->GetSunMergeMap().get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);

	RHI->FrameEnd();
}
