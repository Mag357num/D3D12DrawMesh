#include "stdafx.h"
#include "RenderThread.h"
#include "Engine.h"
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
		});
}

void FRenderThread::UpdateFrameResources(shared_ptr<FScene> Scene)
{
	//RENDER_THREAD([this, Scene]()
	//	{
	//	FrameResourceManager->UpdateFrameResources(Scene, RHI->GetFramIndex());
	//	});
}

void FRenderThread::RenderScene()
{
	//RENDER_THREAD([this]()
	//	{
	//	auto& FrameResource = FrameResourceManager->FrameResources[RHI->GetFramIndex()];
	//	FSimpleRenderer Renderer;
	//	Renderer.RenderScene(RHI, &FrameResource);
	//	RenderCV.notify_all();
	//	});
}

void FRenderThread::WaitForRenderThread()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	RenderCV.wait(Lock);
}
