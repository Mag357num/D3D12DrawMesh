#pragma once
#include "Thread.h"
#include "FrameResourceManager.h"

class FRenderThread : public FTaskThread
{
private:
	shared_ptr<class FFrameResourceManager> FrameResourceManager;
	std::condition_variable RenderCV;
	std::mutex RenderMutex;
	std::atomic_int32_t RenderTaskNum = 0;

	static FRenderThread* GRenderThread;

public:
	FRenderThread();
	virtual void Run() override;

	static void CreateRenderThread();
	static void DestroyRenderThread();

	static FRenderThread* Get();

	void CreateFrameResource(shared_ptr<class FScene> Scene);
	void UpdateFrameRes(FScene* Scene);
	void WaitForRenderThread();

private:
	void DoRender();
};

#define RENDER_THREAD(...) FRenderThread::Get()->AddTask(__VA_ARGS__)