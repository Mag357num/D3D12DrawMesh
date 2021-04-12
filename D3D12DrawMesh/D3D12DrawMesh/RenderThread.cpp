//#include "stdafx.h"
//#include "RenderThread.h"
//#include "Engine.h"
//#include <cassert>
//
//FRenderThread::FRenderThread()
//	: FTaskThread("RenderThread")
//{
//}
//
//void FRenderThread::Run()
//{
//	FRHI::CreateRHI();
//	RHI = FRHI::Get();
//	auto Engine = FEngine::Get();
//	RHI->Init({ 0, 0, Engine->GetWidth(), Engine->GetHeight() });
//
//	FTaskThread::Run();
//}
//
//static FRenderThread* GRenderThread = nullptr;
//
//void FRenderThread::CreateRenderThread()
//{
//	assert(GRenderThread == nullptr);
//	GRenderThread = new FRenderThread();
//}
//
//void FRenderThread::DestroyRenderThread()
//{
//	assert(GRenderThread != nullptr);
//	GRenderThread->Stop();
//	delete GRenderThread;
//	GRenderThread = nullptr;
//}
//
//FRenderThread* FRenderThread::Get()
//{
//	return GRenderThread;
//}
//
//void FRenderThread::CreateResourceForScene(std::shared_ptr<FScene> Scene)
//{
//	RENDER_THREAD([this, Scene]() {
//		FrameResourceManager->CreateRenderResourcesForScene(Scene, RHI->GetFramCount());
//		});
//}
//
//void FRenderThread::UpdateFrameResources(std::shared_ptr<FScene> Scene)
//{
//	RENDER_THREAD([this, Scene]() {
//		FrameResourceManager->UpdateFrameResources(Scene, RHI->GetFramIndex());
//		});
//}
//
//void FRenderThread::RenderScene()
//{
//	RENDER_THREAD([this]() {
//		auto& FrameResource = FrameResourceManager->FrameResources[RHI->GetFramIndex()];
//		FSimpleRenderer Renderer;
//		Renderer.RenderScene(RHI, &FrameResource);
//		--FrameTaskNum;
//		RenderCV.notify_all();
//		});
//}
//
//void FRenderThread::WaitForRenderThread()
//{
//	std::unique_lock<std::mutex> Lock(Mutex);
//	RenderCV.wait(Lock, [this]() { return FrameTaskNum <= RHI->GetFramCount(); });
//	++FrameTaskNum;
//}
