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
	RHI::GDynamicRHI->RHIInit(false, 2, GEngine->GetWidth(), GEngine->GetHeight());

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
	if(FrameResourceManager->FrameResources.size() == 0)
	{
		return;
	}

	FFrameResource& FrameResource = FrameResourceManager->FrameResources[GDynamicRHI->GetFramIndex()];
	FRenderer Renderer;
	Renderer.RenderScene(RHI::GDynamicRHI.get(), &FrameResource);
	--FrameTaskNum;
	RenderCV.notify_all();
}

static FRenderThread* GRenderThread = nullptr;

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

void FRenderThread::CreateResourceForScene(shared_ptr<FScene> Scene)
{
	RENDER_THREAD([this, Scene]()
		{
			FrameResourceManager->CreateFrameResourcesFromScene(Scene, GDynamicRHI->GetFrameCount());
		});
}

void FRenderThread::UpdateFrameResources(FScene* Scene)
{
	RENDER_THREAD([this, Scene]()
		{
			FrameResourceManager->UpdateFrameResources(Scene, GDynamicRHI->GetFramIndex());
		});
}

//void FRenderThread::RenderScene()
//{
//	RENDER_THREAD([this]()
//		{
//			FRenderer Renderer;
//			Renderer.RenderScene(RHI::GDynamicRHI, FrameResourceManager);
//			--FrameTaskNum;
//			RenderCV.notify_all();
//		});
//}

void FRenderThread::WaitForRenderThread()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	RenderCV.wait(Lock, [this]() { return FrameTaskNum < RHI::GDynamicRHI->GetFrameCount(); });
	++FrameTaskNum;
}
