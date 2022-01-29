#include "thread_pool.hxx"

#include "core/utils/log.hxx"

#include "SDL_thread.h"

#include <chrono>
#include <iostream>

class thread_pool_accessor
{
public:
	static std::unique_ptr<thread_task> tryPopTask(thread_pool& pool)
	{
		std::scoped_lock<std::mutex> lock(pool._waitingTasksMutex);
		if (!pool._waitingTasks.empty())
		{
			auto task = std::move(pool._waitingTasks.front());
			pool._waitingTasks.pop_front();
			pool._threadsTaskAwaible = !pool._waitingTasks.empty();
			return task;
		}
		return nullptr;
	}

	static void addCompletedTask(thread_pool& pool, std::unique_ptr<thread_task>&& task)
	{
		std::scoped_lock<std::mutex> lock(pool._completedTasksMutex);
		pool._completedTasks.push_back(std::move(task));
	}
};

static int thread_loop_func(void* data)
{
	thread_pool& pool = *reinterpret_cast<thread_pool*>(data);
	while (pool.getThreadLoopCondition())
	{
		if (pool.getThreadTaskAvaible())
		{
			auto task = thread_pool_accessor::tryPopTask(pool);
			if (task)
			{
				task->doJob();
				if (task->useCompleted())
				{
					thread_pool_accessor::addCompletedTask(pool, std::move(task));
				}
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
	}
	return 0;
}

thread_pool::thread_pool(const char* name, const uint32_t threadCount, const thread_priority priority)
	: _totalTaskCount{0}
{
	const uint32_t num = threadCount > 0 ? threadCount : 1;

	_threads.resize(num);
	for (uint32_t i = 0U; i < num; ++i)
	{
		_threads[i] = SDL_CreateThread(thread_loop_func, name, this);

		const SDL_ThreadPriority sdlPriority = static_cast<SDL_ThreadPriority>(priority);
		SDL_SetThreadPriority(sdlPriority);
	}
}

thread_pool::~thread_pool()
{
	_threadsLoopCondition = false;

	for (auto* thread : _threads)
	{
		int status{0};
		SDL_WaitThread(thread, &status);
		DE_LOG(Verbose, "Thread %p exit with code: %i", thread, status);
	}
}

void thread_pool::tick()
{
	processCompletedTasks();
}

void thread_pool::queueTask(thread_task* task)
{
	task->_id = ++_totalTaskCount;

	std::scoped_lock<std::mutex> guard(_waitingTasksMutex);
	_waitingTasks.push_back(std::unique_ptr<thread_task>(task));
	task->init();
	task->markStart();

	_threadsTaskAwaible = true;
}

uint32_t thread_pool::hardwareConcurrency()
{
	return static_cast<uint32_t>(std::thread::hardware_concurrency());
}

void thread_pool::processCompletedTasks()
{
	while (!_completedTasks.empty())
	{
		std::scoped_lock<std::mutex> lock(_completedTasksMutex);
		auto task = std::move(_completedTasks.front());
		_completedTasks.pop_front();

		task->completed();
		task->markEnd();

		DE_LOG(Info, "Completed async task with id: %u, took: %fs", static_cast<unsigned int>(task->getId()), task->getTaskCompletionTime());
	}
}