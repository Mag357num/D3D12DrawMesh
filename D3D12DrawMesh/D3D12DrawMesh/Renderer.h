#pragma once
#include "stdafx.h"
#include "DXSample.h"
#include "DynamicRHI.h"

using namespace RHI;

class SceneRenderer
{
public:
	SceneRenderer() = default;
	~SceneRenderer() = default;

	void Render(FRHICommandListImmediate& RHICmdList);
};

void SceneRenderer::Render(FRHICommandListImmediate& RHICmdList) // TODO: input the scene binary
{
	// TODO: init the pipeline
	// TODO: load the resource
	// TODO: draw the scene
}