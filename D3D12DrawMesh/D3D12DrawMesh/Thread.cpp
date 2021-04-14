#include "stdafx.h"
#include "Thread.h"

FThread::FThread(const std::string& _Name) : Name(_Name)
{
}

void FThread::Stop()
{
	if (IsRunning)
	{
		IsRunning = false;
	}
	std::unique_lock<std::mutex> Lock(Mutex);
	CVFinished.wait(Lock, [this]() { return IsFinished == true; });
}

void FThread::Start()
{
	if (!IsRunning)
	{
		IsRunning = true;
		IsFinished = false;
		ThreadInstance = std::thread(&FThread::ThreadMain, this);
		ThreadInstance.detach();
	}
}

void FThread::ThreadMain()
{
	SetName();
	Run();
	IsFinished = true;
	CVFinished.notify_all();
}

FTaskThread::FTaskThread(const std::string& _Name) : FThread(_Name)
{
}

void FTaskThread::Run()
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
	}
	Tasks.clear();
}

void FTaskThread::AddTask(TaskLambda&& Lambda)
{
	if (IsRunning)
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		Tasks.push_back(FThreadTask(Lambda));
	}
}

void FThread::SetName()
{
#if defined(_DEBUG)
	/**
		 * Code setting the thread name for use in the debugger.
		 *
		 * http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
		 */
	const uint32 MS_VC_EXCEPTION = 0x406D1388;

	struct THREADNAME_INFO
	{
		uint32 dwType;	   // Must be 0x1000.
		LPCSTR szName;	   // Pointer to name (in user addr space).
		uint32 dwThreadID; // Thread ID (-1=caller thread).
		uint32 dwFlags;	   // Reserved for future use, must be zero.
	};

	THREADNAME_INFO ThreadNameInfo;
	ThreadNameInfo.dwType = 0x1000;
	ThreadNameInfo.szName = Name.c_str();
	ThreadNameInfo.dwThreadID = ::GetCurrentThreadId();
	ThreadNameInfo.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable : 6320 6322)
	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadNameInfo) / sizeof(ULONG_PTR), (ULONG_PTR*)&ThreadNameInfo);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#pragma warning(pop)
#endif
}
