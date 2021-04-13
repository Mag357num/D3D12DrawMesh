#pragma once
#include "RHIResource.h"
#include "FScene.h"
#include "stdafx.h"

using namespace RHI;

class FFrameResourceManager
{
public:
	shared_ptr<FScene> Scene;
public:
	void CreateRenderResourcesForScene(shared_ptr<FScene> SceneParam);
	void UpdateFrameResources();
};