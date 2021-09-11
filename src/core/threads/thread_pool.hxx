#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct SDL_Thread;
class thread_pool;

enum class thread_priority : uint8_t
{
	low,
	normal,
	high,
	timeCritical
};

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

	double getTaskCompletionTime() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(_end - _begin).count() / 1000.0;
	};

protected:
	bool _useCompleted{true};

private:
	void markStart() { _begin = std::chrono::steady_clock::now(); };
	void markEnd() { _end = std::chrono::steady_clock::now(); };

	uint64_t _id{UINT64_MAX};

	std::chrono::steady_clock::time_point _begin;
	std::chrono::steady_clock::time_point _end;
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
	thread_pool(const char* name, const uint32_t threadCount = 0, const thread_priority priority = thread_priority::normal);
	~thread_pool();

	void tick();

	void queueTask(thread_task* task);

	const std::atomic<bool>& getThreadLoopCondition() const { return _threadsLoopCondition; };

	const std::atomic<bool>& getThreadTaskAvaible() const { return _threadsTaskAwaible; };

	static uint32_t hardwareConcurrency();

private:
	void processCompletedTasks();

	std::atomic<bool> _threadsLoopCondition{true};
	std::atomic<bool> _threadsTaskAwaible{false};

	std::mutex _waitingTasksMutex;
	std::queue<thread_task*> _waitingTasks;

	std::mutex _completedTasksMutex;
	std::queue<thread_task*> _completedTasks;

	std::vector<SDL_Thread*> _threads;

	uint64_t _totalTaskCount;
};