#pragma once
#include "DynamicRHI.h"
#include "FrameResourceManager.h"

class FRenderer
{
public:
	virtual ~FRenderer() = default;
	void Render(FDynamicRHI* RHI, FFrameResourceManager* FrameManager);
	void RenderShadow(FDynamicRHI* RHI, FFrameResourceManager* FrameManager);
	void RenderScene(FDynamicRHI* RHI, FFrameResourceManager* FrameManager);
	void RenderPostProcess(FDynamicRHI* RHI, FFrameResourceManager* FrameManager);
};