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
	RHI::GDynamicRHI->RHIInit();

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
	FMultiBufferFrameResource& MFrameRes = FrameResourceManager->GetMultiFrameRes()[FrameIndex];
	FSingleBufferFrameResource& SFrameRes = FrameResourceManager->GetSingleFrameRes();

	FRenderer Renderer;
	Renderer.Render(GDynamicRHI.get(), FrameIndex, SFrameRes, MFrameRes);
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
	RENDER_THREAD([this, Scene]()
		{
			FrameResourceManager->CreateFrameResourcesFromScene(Scene, GDynamicRHI->GetFrameCount());
		});
}

void FRenderThread::UpdateFrameRes(FScene* Scene)
{
	RENDER_THREAD([this, Scene]()
		{
			FrameResourceManager->UpdateFrameResources(Scene, GDynamicRHI->GetFramIndex());
		});
}

//void FRenderThread::UpdateFrameResCamera(FMatrix VP, FVector Eye)
//{
//	RENDER_THREAD([this, VP, Eye]()
//		{
//			FrameResourceManager->UpdateFrameResCamera(VP, Eye, GDynamicRHI->GetFramIndex());
//		});
//}
//
//void FRenderThread::UpdateFrameResPalette(vector<FMatrix> Palette)
//{
//	RENDER_THREAD([this, Palette]()
//		{
//			FrameResourceManager->UpdateFrameResPalette(Palette, GDynamicRHI->GetFramIndex());
//		});
//}

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