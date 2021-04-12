//#pragma once
//
//#include "Thread.h"
//
//class FRenderThread : public FTaskThread
//{
//	std::condition_variable RenderCV;
//	std::mutex RenderMutex;
//	std::atomic_int32_t FrameTaskNum = 0;
//
//public:
//	FRenderThread();
//	virtual void Run() override;
//
//	static void CreateRenderThread();
//	static void DestroyRenderThread();
//
//	static FRenderThread* Get();
//
//	void CreateResourceForScene(std::shared_ptr<class FScene> Scene);
//	void UpdateFrameResources(std::shared_ptr<class FScene> Scene);
//	void RenderScene();
//	void WaitForRenderThread();
//};
//
//#define RENDER_THREAD(...) FRenderThread::Get()->AddTask(__VA_ARGS__)