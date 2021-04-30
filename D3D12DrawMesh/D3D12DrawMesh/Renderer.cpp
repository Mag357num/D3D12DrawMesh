#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes)
{
	RHI->FrameBegin();

	// shadow pass
	RHI->ClearDepthStencil(FrameRes->ShadowMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(FrameRes->ShadowMapSize), static_cast<float>(FrameRes->ShadowMapSize), 0.f, 1.f);
	RHI->SetScissor(0, 0, FrameRes->ShadowMapSize, FrameRes->ShadowMapSize);
	RHI->SetRenderTarget(0, nullptr, FrameRes->ShadowMap->DsvHandle.get());
	for (auto i : FrameRes->MeshActorFrameReses)
	{
		// use shadow pso
		RHI->ChoosePipelineState(i.MeshRes->ShadowMat->PSO.get());

		// root signature
		RHI->SetShaderInput(FPassType::SHADOW_PT, i.MeshRes->ShadowMat.get(), FrameRes);

		// set mesh
		RHI->DrawMesh(i.Mesh.get());
	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// scene color pass
	RHI->ClearRenderTarget(FrameRes->SceneColorMap->RtvHandle.get());
	RHI->SetRenderTarget(1, FrameRes->SceneColorMap->RtvHandle.get(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
	for (auto i : FrameRes->MeshActorFrameReses)
	{
		// pso
		RHI->ChoosePipelineState(i.MeshRes->SceneColorMat->PSO.get()); // use HDR pso

		// root signature
		RHI->SetShaderInput(FPassType::SCENE_COLOR_PT, i.MeshRes->SceneColorMat.get(), FrameRes);

		// set mesh
		RHI->DrawMesh(i.Mesh.get());
	}
	RHI->TransitTextureState(FrameRes->SceneColorMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// bloom setup
	RHI->ClearRenderTarget(FrameRes->BloomSetupMap->RtvHandle.get());
	RHI->SetRenderTarget(1, FrameRes->BloomSetupMap->RtvHandle.get(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 4), static_cast<float>(RHI->GetHeight() / 4), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth() / 4, RHI->GetHeight() / 4);
	RHI->ChoosePipelineState(FrameRes->PastProcessTriangleRes->BloomSetupMat->PSO.get());
	RHI->SetShaderInput(FPassType::BLOOM_SETUP_PT, FrameRes->PastProcessTriangleRes->BloomSetupMat.get(), FrameRes);
	RHI->DrawMesh(FrameRes->PastProcessTriangle.get());
	RHI->TransitTextureState(FrameRes->BloomSetupMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// bloom down 1/4->1/8
	RHI->ClearRenderTarget(FrameRes->BloomDownMap8->RtvHandle.get());
	RHI->SetRenderTarget(1, FrameRes->BloomDownMap8->RtvHandle.get(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 8), static_cast<float>(RHI->GetHeight() / 8), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth() / 8, RHI->GetHeight() / 8);
	RHI->ChoosePipelineState(FrameRes->PastProcessTriangleRes->BloomDownMat[0]->PSO.get());
	RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->PastProcessTriangleRes->BloomDownMat[0].get(), FrameRes);
	RHI->DrawMesh(FrameRes->PastProcessTriangle.get());
	RHI->TransitTextureState(FrameRes->BloomDownMap8.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// bloom down 1/8->1/16
	RHI->ClearRenderTarget(FrameRes->BloomDownMap16->RtvHandle.get());
	RHI->SetRenderTarget(1, FrameRes->BloomDownMap16->RtvHandle.get(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 16), static_cast<float>(RHI->GetHeight() / 16), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth() / 16, RHI->GetHeight() / 16);
	RHI->ChoosePipelineState(FrameRes->PastProcessTriangleRes->BloomDownMat[1]->PSO.get());
	RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->PastProcessTriangleRes->BloomDownMat[1].get(), FrameRes);
	RHI->DrawMesh(FrameRes->PastProcessTriangle.get());
	RHI->TransitTextureState(FrameRes->BloomDownMap16.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// bloom down 1/16->1/32
	RHI->ClearRenderTarget(FrameRes->BloomDownMap32->RtvHandle.get());
	RHI->SetRenderTarget(1, FrameRes->BloomDownMap32->RtvHandle.get(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 32), static_cast<float>(RHI->GetHeight() / 32), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth() / 32, RHI->GetHeight() / 32);
	RHI->ChoosePipelineState(FrameRes->PastProcessTriangleRes->BloomDownMat[2]->PSO.get());
	RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->PastProcessTriangleRes->BloomDownMat[2].get(), FrameRes);
	RHI->DrawMesh(FrameRes->PastProcessTriangle.get());
	RHI->TransitTextureState(FrameRes->BloomDownMap32.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// bloom down 1/32->1/64
	RHI->ClearRenderTarget(FrameRes->BloomDownMap64->RtvHandle.get());
	RHI->SetRenderTarget(1, FrameRes->BloomDownMap64->RtvHandle.get(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth() / 64), static_cast<float>(RHI->GetHeight() / 64), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth() / 64, RHI->GetHeight() / 64);
	RHI->ChoosePipelineState(FrameRes->PastProcessTriangleRes->BloomDownMat[3]->PSO.get());
	RHI->SetShaderInput(FPassType::BLOOM_DOWN_PT, FrameRes->PastProcessTriangleRes->BloomDownMat[3].get(), FrameRes);
	RHI->DrawMesh(FrameRes->PastProcessTriangle.get());
	RHI->TransitTextureState(FrameRes->BloomDownMap64.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);



	// bloom up

	

	// tonemapping output
	RHI->ClearRenderTarget(RHI->GetBackBufferHandle());
	RHI->SetRenderTarget(1, RHI->GetBackBufferHandle(), FrameRes->DepthStencilMap->DsvHandle.get());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
	RHI->ChoosePipelineState(FrameRes->PastProcessTriangleRes->ToneMappingMat->PSO.get());
	RHI->SetShaderInput(FPassType::TONEMAPPING_PT, FrameRes->PastProcessTriangleRes->ToneMappingMat.get(), FrameRes);
	RHI->DrawMesh(FrameRes->PastProcessTriangle.get());
	
	// transition
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE);
	RHI->TransitTextureState(FrameRes->SceneColorMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->BloomSetupMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->BloomDownMap8.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->BloomDownMap16.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->BloomDownMap32.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->TransitTextureState(FrameRes->BloomDownMap64.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);

	RHI->FrameEnd();
}
