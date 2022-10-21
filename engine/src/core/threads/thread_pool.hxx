#pragma once

#include "dreco.hxx"
#include "thread_task.hxx"

#include <atomic>
#include <map>
#include <mutex>
#include <string_view>
#include <thread>

struct SDL_Thread;
struct thread_task;

class thread_pool
{
	enum class priority : uint8_t
	{
		low,
		normal,
		high,
		timeCritical
	};
	enum class task_state : uint8_t
	{
		uninitialized, // never touched by threads
		waiting,	   // waiting for execution
		procesing,	   // is in execution
		processed,	   // exection complete
		done,		   // everything done
	};
	struct loop_info
	{
		thread_pool* _pool;
		priority _priority;
	};

public:
	thread_pool(const std::string_view name, const uint32_t threadCount, const priority priority = priority::normal);
	~thread_pool();

	void tick(uint64_t frameCount);

	template <typename Task, class... Args>
	thread_task::shared queueTask(Args&&... args);

	template <typename Task>
	thread_task::shared queueTask(Task&& task);

	void setCleanupFrame(uint64_t inValue);

	bool getLoopCondition() const;

	priority getPriority() const;

	static uint32_t hardwareConcurrency();

	uint64_t makeTaskId();

	thread_task::shared findTask(const uint64_t taskId);

private:
	static int threadsLoop(void* data);

	thread_task* beginTaskProcessing();
	void endProcessingTask(thread_task* task);

	std::mutex _tasksMutex{};
	std::map<thread_task::shared, std::atomic<task_state>> _tasks{};

	std::vector<SDL_Thread*> _threads{};

	uint64_t _totalTaskCount{};

	uint64_t _cleanupFrame{5000};

	std::atomic<bool> _loopCondition{true};

	priority _priority{priority::normal};
};

template <typename Task, class... Args>
thread_task::shared thread_pool::queueTask(Args&&... args)
{
	static_assert(std::is_base_of<thread_task, Task>::value, "Task must be derived from thread_task");

	auto task = thread_task::makeNew<Task>(makeTaskId(), std::forward<Args>(args)...);

	std::scoped_lock<std::mutex> guard(_tasksMutex);
	return _tasks.emplace(thread_task::shared(task), task_state::uninitialized).first->first;
}

template <typename Task>
thread_task::shared thread_pool::queueTask(Task&& task)
{
	std::scoped_lock<std::mutex> guard(_tasksMutex);
	return _tasks.emplace(thread_task::shared(std::forward<Task>(task)), task_state::uninitialized).first->first;
}