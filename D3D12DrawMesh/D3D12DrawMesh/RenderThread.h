#pragma once
#include "Thread.h"
#include "FrameResourceManager.h"

class FRenderThread : public FTaskThread
{
	shared_ptr<class FFrameResourceManager> FrameResourceManager;
	std::condition_variable RenderCV;
	std::mutex RenderMutex;

public:
	FRenderThread();
	virtual void Run() override;

	static void CreateRenderThread();
	static void DestroyRenderThread();

	static FRenderThread* Get();

	void CreateResourceForScene(shared_ptr<class FScene> Scene);
	void UpdateFrameResources(shared_ptr<class FScene> Scene);
	void RenderScene();
	void WaitForRenderThread();
};

#define RENDER_THREAD(...) FRenderThread::Get()->AddTask(__VA_ARGS__)