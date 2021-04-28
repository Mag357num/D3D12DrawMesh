#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes)
{
	RHI->FrameBegin();

	// shadow pass
	RHI->ClearDepthStencil(FrameRes->ShadowMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(FrameRes->ShadowMapSize), static_cast<float>(FrameRes->ShadowMapSize), 0.f, 1.f);
	RHI->SetScissor(0, 0, FrameRes->ShadowMapSize, FrameRes->ShadowMapSize);
	RHI->SetRenderTarget(FPassType::SHADOW_PASS, FrameRes->ShadowMap.get());

	for (auto i : FrameRes->MeshActorFrameResources)
	{
		// use shadow pso
		RHI->SetRasterizer(i.MeshResToRender->ShadowRas.get());

		// root signature
		RHI->SetShaderInput(FPassType::SHADOW_PASS, i.MeshResToRender.get(), FrameRes);

		// set mesh
		RHI->SetMeshBuffer(i.MeshToRender.get());
	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// base pass
	RHI->ClearDepthStencil(FrameRes->DepthStencilMap.get());
	RHI->SetViewport(0.0f, 0.0f, static_cast<float>(RHI->GetWidth()), static_cast<float>(RHI->GetHeight()), 0.f, 1.f);
	RHI->SetScissor(0, 0, RHI->GetWidth(), RHI->GetHeight());
	RHI->SetRenderTarget(FPassType::BASE_PASS, FrameRes->DepthStencilMap.get());

	for (auto i : FrameRes->MeshActorFrameResources)
	{
		// pso
		RHI->SetRasterizer(i.MeshResToRender->BaseRas.get());

		// root signature
		RHI->SetShaderInput(FPassType::BASE_PASS, i.MeshResToRender.get(), FrameRes);

		// set mesh
		RHI->SetMeshBuffer(i.MeshToRender.get());
	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE);

	RHI->FrameEnd();
}
