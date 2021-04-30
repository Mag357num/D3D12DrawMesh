#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes)
{
	RHI->FrameBegin();

	// shadow pass
	RHI->ClearDepthStencil(FrameRes->ShadowMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(FrameRes->ShadowMapSize), static_cast<float>(FrameRes->ShadowMapSize), 0.f, 1.f);
	RHI->SetScissor(0, 0, FrameRes->ShadowMapSize, FrameRes->ShadowMapSize);
	RHI->SetRenderTarget(FPassType::SHADOW_PT, nullptr, FrameRes->ShadowMap.get());
	for (auto i : FrameRes->MeshActorFrameReses)
	{
		// use shadow pso
		RHI->ChoosePipelineState(i.MeshRes->ShadowPassPso.get());

		// root signature
		RHI->SetShaderInput(FPassType::SHADOW_PT, i.MeshRes.get(), FrameRes);

		// set mesh
		RHI->DrawMesh(i.Mesh.get());
	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// scene color pass
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
	RHI->SetRenderTarget(FPassType::SCENE_COLOR_PT, FrameRes->SceneColorMap.get(), FrameRes->DepthStencilMap.get());
	for (auto i : FrameRes->MeshActorFrameReses)
	{
		// pso
		RHI->ChoosePipelineState(i.MeshRes->SceneColorPso.get()); // use HDR pso

		// root signature
		RHI->SetShaderInput(FPassType::SCENE_COLOR_PT, i.MeshRes.get(), FrameRes);

		// set mesh
		RHI->DrawMesh(i.Mesh.get());
	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE);
	RHI->TransitTextureState(FrameRes->SceneColorMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// bloom setup

	// bloom down

	// bloom up

	// combine

	// tonemapping output
	RHI->ClearRenderTarget(RHI->GetBackBufferHandle());
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
	RHI->SetRenderTarget(FPassType::LDR_OUTPUT_RT_PT, nullptr, FrameRes->DepthStencilMap.get());
	RHI->ChoosePipelineState(FrameRes->PostProcessTriangleRes->OutputPassPso.get()); // pso
	RHI->SetShaderInput(FPassType::LDR_OUTPUT_RT_PT, FrameRes->PostProcessTriangleRes.get(), FrameRes); // root signature
	RHI->DrawMesh(FrameRes->PostProcessTriangle.get()); // set mesh 


	RHI->TransitTextureState(FrameRes->SceneColorMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET);
	RHI->FrameEnd();
}
