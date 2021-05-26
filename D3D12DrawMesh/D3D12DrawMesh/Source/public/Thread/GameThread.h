#pragma once
#include "Thread.h"

class FGameThread : public FThread
{
private:
	//shared_ptr<class FFrameResourceManager> FrameResourceManager;
	//std::condition_variable RenderCV;
	//std::mutex RenderMutex;
	//std::atomic_int32_t RenderTaskNum = 0;

	static FGameThread* GGameThread;

public:
	FGameThread();
	virtual void Run() override;

	static void CreateRenderThread();
	static void DestroyRenderThread();

	static FGameThread* Get() { return GGameThread; }

	void CreateFrameResource(shared_ptr<class FScene> Scene);
	void UpdateFrameRes(FScene* Scene);
	void WaitForRenderThread();

private:
	void DoRender();
};