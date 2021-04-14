#pragma once
#include "Thread.h"
#include "FrameResourceManager.h"

class FRenderThread : public FTaskThread
{
	shared_ptr<class FFrameResourceManager> FrameResourceManager;
	std::condition_variable RenderCV;
	std::mutex RenderMutex;
	std::atomic_int32_t FrameTaskNum = 0;

public:
	FRenderThread();
	virtual void Run() override;
	virtual void DoRender() override;

	static void CreateRenderThread();
	static void DestroyRenderThread();

	static FRenderThread* Get();

	void CreateResourceForScene(shared_ptr<class FScene> Scene);
	void UpdateFrameResources();
	void RenderScene();
	void WaitForRenderThread();
};

#define RENDER_THREAD(...) FRenderThread::Get()->AddTask(__VA_ARGS__)