#include "thread_pool.hxx"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>

std::atomic<bool> threadLoopCondition{true};
std::atomic<bool> threadTaskAwaible{false};

std::queue<thread_task*> waitingTasks;
std::mutex waitingTasksMutex;

std::queue<thread_task*> completedTasks;
std::mutex completedTasksMutex;

static bool thread_pop_task(thread_task** task)
{
	*task = std::move(waitingTasks.front());
	waitingTasks.pop();

	threadTaskAwaible = !waitingTasks.empty();

	return *task != nullptr;
}

static void thread_complete_task(thread_task* const task)
{
	std::lock_guard<std::mutex> guard(completedTasksMutex);
	completedTasks.push(task);
}

static void thread_loop()
{
	while (threadLoopCondition)
	{
		if (threadTaskAwaible && waitingTasksMutex.try_lock())
		{
			thread_task* task{nullptr};
			const bool result = thread_pop_task(&task);
			waitingTasksMutex.unlock();

			if (result)
			{
				task->doJob();
				thread_complete_task(task);
				continue;
			}
		}
		std::this_thread::yield();
	}
}

thread_pool::thread_pool()
	: _totalTaskCount{0}
{
	const auto num = std::thread::hardware_concurrency() / 2;

	_threads.resize(num);
	for (auto i = 0U; i < num; ++i)
	{
		_threads[i] = std::thread(thread_loop);
	}
}

thread_pool::~thread_pool()
{
	threadLoopCondition = false;
	for (auto& thread : _threads)
	{
		thread.join();
	}
}

void thread_pool::tick(const double& deltaTime)
{
    processCompletedTasks();
}

void thread_pool::queueTask(thread_task* task)
{
	task->id = ++_totalTaskCount;

	std::lock_guard<std::mutex> guard(waitingTasksMutex);
	waitingTasks.push(task);

	task->init();

	threadTaskAwaible = true;
}

void thread_pool::processCompletedTasks()
{
	while (!completedTasks.empty())
	{
		std::lock_guard<std::mutex> guard(completedTasksMutex);

		thread_task* task = std::move(completedTasks.front());
		completedTasks.pop();

		task->compeleted();

		std::cout << "thread_pool: completed task with id: " << task->getId() << std::endl;

		delete task;
	}
}