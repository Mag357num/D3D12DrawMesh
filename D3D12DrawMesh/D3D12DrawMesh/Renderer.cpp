#include "Renderer.h"

void FRenderer::RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes)
{
	RHI->FrameBegin();
	RHI->DrawFrame(FrameRes);
	RHI->FrameEnd();
}
