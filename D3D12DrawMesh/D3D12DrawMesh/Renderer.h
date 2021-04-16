#pragma once
#include "DynamicRHI.h"
#include "FrameResourceManager.h"

class FRenderer
{
public:
	virtual ~FRenderer() = default;

	void RenderScene(FDynamicRHI* RHI, FFrameResource* FrameRes);
};