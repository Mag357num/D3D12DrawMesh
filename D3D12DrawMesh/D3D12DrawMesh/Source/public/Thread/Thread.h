#pragma once
#include "stdafx.h"

class FThread
{
	void ThreadMain();

protected:
	std::string Name;
	std::atomic_bool IsRunning = false;
	std::atomic_bool IsFinished = false;
	std::mutex Mutex;
	std::thread ThreadInstance;
	std::condition_variable CVFinished;

public:
	FThread(const std::string& _Name);
	virtual ~FThread() = default;
	virtual void Start();
	virtual void Stop();

	virtual void Run() = 0;

	void SetName();
};

using TaskLambda = std::function<void()>;
class FThreadTask
{
private:
	TaskLambda Task;

public:
	FThreadTask(TaskLambda Lambda) : Task(Lambda){};

	void DoTask()
	{
		if (Task)
		{
			Task();
		}
	}
};

class FTaskThread : public FThread
{
protected:
	std::deque<FThreadTask> Tasks;

public:
	FTaskThread(const std::string& _Name);
	virtual void Run() override;

	virtual void AddTask(TaskLambda&& Lambda);
};
