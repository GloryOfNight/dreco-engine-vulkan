#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <vector>

class thread_pool;

struct thread_task
{
	friend thread_pool;

	virtual ~thread_task(){};

	virtual void init() = 0;

	virtual void doJob() = 0;

	virtual void compeleted() = 0;

	uint64_t getId() const
	{
		return id;
	}

private:
	uint64_t id{UINT64_MAX};
};

class thread_pool
{
public:
	thread_pool();
	~thread_pool();

	void tick(const double& deltaTime);

	void queueTask(thread_task* task);

private:
    void processCompletedTasks();

	std::vector<std::thread> _threads;

	uint64_t _totalTaskCount;
};