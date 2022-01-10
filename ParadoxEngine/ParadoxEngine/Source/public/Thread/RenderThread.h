#pragma once
#include "Thread.h"
#include "FrameResourceManager.h"

class FRenderThread : public FTaskThread
{
private:
	shared_ptr<class FFrameResourceManager> FrameResourceManager;
	std::condition_variable RenderCV;
	std::mutex RenderMutex;
	std::atomic_int32_t FrameTaskNum = 0;

	static shared_ptr<FRenderThread> GRenderThread;

public:
	FRenderThread();
	virtual void Run() override;

	static void CreateRenderThread();
	static void DestroyRenderThread();

	static FRenderThread* Get();

	void CreateFrameResource(shared_ptr<class FScene> Scene);

	void UpdateFrameRes(FScene* Scene);
	//void UpdateFrameResCamera(FMatrix VP, FVector Eye);
	//void UpdateFrameResPalette(vector<FMatrix> Palette);

	void WaitForRenderer();

private:
	void DoRender();
};

#define ENQUEUE_RENDER_COMMAND(...) FRenderThread::Get()->AddTask(__VA_ARGS__)