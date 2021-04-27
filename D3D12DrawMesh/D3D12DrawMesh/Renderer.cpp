#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes)
{
	RHI->FrameBegin();


	for (auto i : FrameRes->MeshActorFrameResources)
	{
		//FDX12MeshRes* DX12MeshRes = dynamic_cast<FDX12MeshRes*>(i.MeshResToRender.get());
		//FDX12CB* ShadowCB = dynamic_cast<FDX12CB*>(i.MeshResToRender->ShadowCB.get()); // shadow cb

		// viewport scissorect
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(FrameRes->ShadowMapSize), static_cast<float>(FrameRes->ShadowMapSize), 0.f, 1.f);
		RHI->SetScissor(0, 0, FrameRes->ShadowMapSize, FrameRes->ShadowMapSize);

		// set render target to nullptr, dsv to shadow texture
		RHI->SetRenderTarget(nullptr, FrameRes->ShadowMap.get());

		// use shadow pso
		GDynamicRHI->SetRasterizer(i.MeshResToRender->ShadowRas.get());

		// root signature
		CommandLists[0].CommandList->SetGraphicsRootSignature(DX12MeshRes->RootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { CBVSRVHeap.Get(), SamplerHeap.Get() };
		CommandLists[0].CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(0, ShadowCB->GPUHandleInHeap); // cb
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(2, DX12Sampler->SamplerHandle); // sampler

		// set mesh
		GDynamicRHI->SetMeshBuffer(i.MeshToRender.get());
	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	for (auto i : FrameRes->MeshActorFrameResources)
	{
		//FDX12Mesh* DX12Mesh = dynamic_cast<FDX12Mesh*>(MeshActor.MeshToRender.get());
		//FDX12MeshRes* DX12MeshRes = dynamic_cast<FDX12MeshRes*>(MeshActor.MeshResToRender.get());
		//FDX12CB* DX12CB = dynamic_cast<FDX12CB*>(MeshActor.MeshResToRender->BaseCB.get()); // base pass cb

		// viewport scissorect
		RHI->SetViewport(0.0f, 0.0f, static_cast<float>(TODO), static_cast<float>(TODO), 0.f, 1.f);
		RHI->SetScissor(0, 0, TODO, TODO);

		// rendertarget
		RHI->SetRenderTarget(FrameRes->RenderTargets[RHI->GetBackBufferIndex()].get(), FrameRes->DepthStencilMap.get());

		// pso
		GDynamicRHI->SetRasterizer(MeshActor.MeshResToRender->BaseRas.get());

		// root signature
		CommandLists[0].CommandList->SetGraphicsRootSignature(DX12MeshRes->RootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { CBVSRVHeap.Get(), SamplerHeap.Get() };
		CommandLists[0].CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(0, DX12CB->GPUHandleInHeap); // cb
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(1, DX12ShadowMap->SrvHandle); // shadow map texture
		CommandLists[0].CommandList->SetGraphicsRootDescriptorTable(2, DX12Sampler->SamplerHandle); // sampler

		// set mesh
		GDynamicRHI->SetRasterizer(i.MeshResToRender->BaseRas.get());

	}
	RHI->TransitTextureState(FrameRes->ShadowMap.get(), FRESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, FRESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE);





	RHI->FrameEnd();
}
