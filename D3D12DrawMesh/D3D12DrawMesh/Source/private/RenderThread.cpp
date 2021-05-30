#include "RenderThread.h"
#include "Engine.h"
#include "Renderer.h"
#include <cassert>

FRenderThread::FRenderThread()
	: FTaskThread("RenderThread")
{
	FrameResourceManager = std::make_shared<FFrameResourceManager>();
}

void FRenderThread::Run()
{
	RHI::GDynamicRHI->CreateRHI();
	RHI::GDynamicRHI->RHIInit(false, RHI::BACKBUFFER_NUM, GEngine->GetWidth(), GEngine->GetHeight());

	while (IsRunning)
	{
		FThreadTask* Task = nullptr;
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			if (!Tasks.empty())
			{
				Task = &Tasks.front();
			}
		}
		if (Task != nullptr)
		{
			Task->DoTask();
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				Tasks.pop_front();
			}
		}

		DoRender();
	}
	Tasks.clear();
}

void FRenderThread::DoRender()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	RenderCV.wait(Lock, [this]() { return RenderTaskNum > 0; });

	FRenderer Renderer;
	Renderer.RenderScene(RHI::GDynamicRHI.get(), FrameResourceManager.get());
	--RenderTaskNum;
	RenderCV.notify_all();
}

FRenderThread* FRenderThread::GRenderThread = nullptr;

void FRenderThread::CreateRenderThread()
{
	assert(GRenderThread == nullptr);
	GRenderThread = new FRenderThread();
}

void FRenderThread::DestroyRenderThread()
{
	assert(GRenderThread != nullptr);
	GRenderThread->Stop();
	delete GRenderThread;
	GRenderThread = nullptr;
}

FRenderThread* FRenderThread::Get()
{
	return GRenderThread;
}

void FRenderThread::CreateFrameResource(shared_ptr<TScene> Scene)
{
	RENDER_THREAD([this, Scene]()
		{
			FrameResourceManager->CreateFrameResourcesFromScene(Scene, GDynamicRHI->GetFrameNum());
		});
}

void FRenderThread::UpdateFrameRes(TScene* Scene)
{
	RENDER_THREAD([this, Scene]()
		{
			FrameResourceManager->UpdateFrameResources(Scene, GDynamicRHI->GetCurrentFramIndex());
		});
}

void FRenderThread::WaitForRenderThread()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	RenderCV.wait(Lock, [this]() { return RenderTaskNum < static_cast<int32_t>(RHI::GDynamicRHI->GetFrameNum()); });
	++RenderTaskNum;
	RenderCV.notify_all();
}