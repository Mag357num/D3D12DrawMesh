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
	const uint32& FrameIndex = GDynamicRHI->GetFramIndex();

	FRenderer Renderer;
	Renderer.Render(GDynamicRHI.get(), FrameIndex, FrameResourceManager.get());
	--FrameTaskNum;
	RenderCV.notify_all();
}

shared_ptr<FRenderThread> FRenderThread::GRenderThread = nullptr;

void FRenderThread::CreateRenderThread()
{
	assert(GRenderThread == nullptr);
	GRenderThread = make_shared<FRenderThread>();
}

void FRenderThread::DestroyRenderThread()
{
	assert(GRenderThread != nullptr);
	GRenderThread->Stop();
	GRenderThread = nullptr;
}

FRenderThread* FRenderThread::Get()
{
	return GRenderThread.get();
}

void FRenderThread::CreateFrameResource(shared_ptr<FScene> Scene)
{
	ENQUEUE_RENDER_COMMAND([this, Scene]()
		{
			FrameResourceManager->CreateActorsFrameRes(Scene, GDynamicRHI->GetFrameCount());
		});
}

void FRenderThread::UpdateFrameRes(FScene* Scene)
{
	ENQUEUE_RENDER_COMMAND([this, Scene]()
		{
			FrameResourceManager->UpdateFrameResources(Scene, GDynamicRHI->GetFramIndex());
		});
}

void FRenderThread::WaitForRenderer()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	RenderCV.wait(Lock, [this]()
		{
			return FrameTaskNum < static_cast<int32_t>(RHI::GDynamicRHI->GetFrameCount());
		});
	++FrameTaskNum;
	RenderCV.notify_all();
}