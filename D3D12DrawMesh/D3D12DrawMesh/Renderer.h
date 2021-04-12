#pragma once
#include "DynamicRHI.h"
#include "FrameResourceManager.h"

class FRenderer
{
public:
	virtual ~FRenderer() = default;

	void RenderScene(shared_ptr<FDynamicRHI> RHI, shared_ptr<FFrameResourceManager> FrameRes);
};