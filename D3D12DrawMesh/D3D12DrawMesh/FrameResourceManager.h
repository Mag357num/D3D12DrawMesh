#pragma once
#include "RHIResource.h"
#include "FScene.h"
#include "stdafx.h"

using namespace RHI;

struct FFrameResourceManager
{
	shared_ptr<FScene> Scene;

	void CreateRenderResourcesForScene(shared_ptr<FScene> SceneParam);
	void UpdateFrameResources();
};