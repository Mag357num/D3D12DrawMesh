#include "GameThread.h"

void FGameThread::CreateRenderThread()
{
	assert(GGameThread == nullptr);
	GGameThread = new FGameThread();
}

void FGameThread::DestroyRenderThread()
{
	assert(GGameThread != nullptr);
	GGameThread->Stop();
	delete GGameThread;
	GGameThread = nullptr;
}

