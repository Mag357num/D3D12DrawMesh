#include "Renderer.h"

void FRenderer::RenderScene(shared_ptr<FDynamicRHI> RHI, shared_ptr<FFrameResourceManager> FrameRes)
{
	RHI->FrameBegin();
	RHI->DrawScene(*FrameRes->Scene);
	RHI->FrameEnd();
}
