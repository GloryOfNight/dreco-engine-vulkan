#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool;

struct thread_task
{
	friend thread_pool;

	virtual ~thread_task() = default;

	// in-sync with main
	virtual void init() = 0;
	
	// async
	virtual void doJob() = 0;

	// in-sync with main
	virtual void compeleted() = 0;

	uint64_t getId() const { return _id; }

	bool useCompleted() const { return _useCompleted; }

protected:
	bool _useCompleted{true};

private:
	uint64_t _id{UINT64_MAX};
};

// empty thread task for testing
struct empty_thread_task : public thread_task
{
	void init() override{};
	void doJob() override{};
	void compeleted() override{};
};

class thread_pool
{
	friend class thread_pool_accessor;

public:
	thread_pool();
	~thread_pool();

	void tick();

	void queueTask(thread_task* task);

	const std::atomic<bool>& getThreadLoopCondition() const { return _threadsLoopCondition; };

	const std::atomic<bool>& getThreadTaskAvaible() const { return _threadsTaskAwaible; };

private:
	void processCompletedTasks();

	std::atomic<bool> _threadsLoopCondition{true};
	std::atomic<bool> _threadsTaskAwaible{false};

	std::mutex _waitingTasksMutex;
	std::queue<thread_task*> _waitingTasks;

	std::mutex _completedTasksMutex;
	std::queue<thread_task*> _completedTasks;

	std::vector<std::thread> _threads;

	uint64_t _totalTaskCount;
};