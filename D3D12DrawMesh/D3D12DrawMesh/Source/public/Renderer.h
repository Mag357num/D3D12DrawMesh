#pragma once
#include "DynamicRHI.h"
#include "FrameResourceManager.h"

class FRenderer
{
public:
	virtual ~FRenderer() = default;
	void Render(FDynamicRHI* RHI, const uint32& FrameIndex, FFrameResourceManager* FRManager);
	void RenderShadow(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes);
	void RenderScene(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes);
	void RenderPostProcess(FDynamicRHI* RHI, const uint32& FrameIndex, FSingleBufferFrameResource& SFrameRes, FMultiBufferFrameResource& MFrameRes);
};