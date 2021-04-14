#pragma once
#include "RHIResource.h"
#include "FScene.h"
#include "stdafx.h"

using namespace RHI;

class FFrameResourceManager // TODO: this now is actually ResourceManager, nothing to do with Frame Resource
{
public:
	shared_ptr<FScene> Scene;
public:
	void CreateRenderResourcesForScene(shared_ptr<FScene> SceneParam);
	void UpdateFrameResources();
};