#include "thread_pool.hxx"

#include <atomic>
#include <mutex>
#include <chrono>
#include <iostream>

std::atomic<bool> threadLoopCondition = true;
std::atomic<bool> threadTaskAwaible = false;

std::queue<thread_task> waitingTasks;
std::mutex waitingTasksMutex;

std::queue<thread_task> completedTasks;
std::mutex completedTasksMutex;

static thread_task thread_pop_task()
{
    std::lock_guard<std::mutex> guard(waitingTasksMutex);

    const auto task = waitingTasks.front();
    waitingTasks.pop();

    threadTaskAwaible = waitingTasks.size() > 0;

    return task;
}

static void thread_complete_task(const thread_task& task)
{
    std::lock_guard<std::mutex> guard(completedTasksMutex);

    completedTasks.push(task);
}

static void thread_loop()
{
	while (threadLoopCondition)
	{
        if (threadTaskAwaible)
        {
            auto task = thread_pop_task();
            task.job();
            thread_complete_task(task);
        }
        else 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
	}
}

thread_pool::thread_pool()
{
	const auto num = std::thread::hardware_concurrency();

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

void thread_pool::tick()
{
    std::lock_guard<std::mutex> guard(completedTasksMutex);

	while (completedTasks.size() > 0)
	{
		completedTasks.front().callback();
		completedTasks.pop();

        std::cout << "completed task" << std::endl;
	}
}

void thread_pool::queueTask(const thread_task& task)
{
    std::lock_guard<std::mutex> guard(waitingTasksMutex);
	waitingTasks.push(task);

    threadTaskAwaible = true;
}