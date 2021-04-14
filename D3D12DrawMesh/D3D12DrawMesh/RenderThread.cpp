#include "stdafx.h"
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
	FTaskThread::Run();
}

void FRenderThread::DoRender()
{
	FRenderer Renderer;
	Renderer.RenderScene(RHI::GDynamicRHI, FrameResourceManager);
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
			FrameResourceManager->CreateRenderResourcesForScene(Scene);
			GDynamicRHI->SyncFrame();
		});
}

void FRenderThread::UpdateFrameResources()
{
	RENDER_THREAD([this]()
		{
			FrameResourceManager->UpdateFrameResources();
		});
}

void FRenderThread::RenderScene()
{
	RENDER_THREAD([this]()
		{
			FRenderer Renderer;
			Renderer.RenderScene(RHI::GDynamicRHI, FrameResourceManager);
			--FrameTaskNum;
			RenderCV.notify_all();
		});
}

void FRenderThread::WaitForRenderThread()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	RenderCV.wait(Lock, [this]() { return FrameTaskNum < RHI::GDynamicRHI->GetFramCount(); });
	++FrameTaskNum;
}
