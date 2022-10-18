#pragma once

#include "dreco.hxx"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
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
	using callback = std::function<void(thread_task*)>;

	thread_task() = default;
	virtual ~thread_task() = default;

	// in-sync with main
	virtual void init() = 0;

	// async
	virtual void doJob() = 0;

	// in-sync with main
	virtual void completed()
	{
		for (const auto& callback : _callbacks)
		{
			try
			{
				callback(this);
			}
			catch (std::bad_function_call badFuncCall)
			{
				DE_LOG(Error, "%s: BadFunctionCall: %s", __FUNCTION__, badFuncCall.what());
			}
		}
	}

	template<typename T, typename F>
	void bindCallback(T* obj, F func) 
	{
		bindCallback(std::bind(func, obj, std::placeholders::_1));
	}

	void bindCallback(callback&& inCallback)
	{
		_callbacks.emplace_back(inCallback);
	}

	uint64_t getId() const { return _id; };

	bool useCompleted() const { return _useCompleted; };

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

	std::vector<callback> _callbacks;
};

// empty thread task for testing
struct empty_thread_task : public thread_task
{
	void init() override{};
	void doJob() override{};
	void completed() override{};
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
	std::deque<std::unique_ptr<thread_task>> _waitingTasks;

	std::mutex _completedTasksMutex;
	std::deque<std::unique_ptr<thread_task>> _completedTasks;

	std::vector<SDL_Thread*> _threads;

	uint64_t _totalTaskCount;
};