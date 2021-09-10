#include "thread_pool.hxx"

#include <chrono>
#include <iostream>

class thread_pool_accessor
{
public:
	static thread_task* tryPopTask(thread_pool& pool)
	{
		std::scoped_lock<std::mutex> lock(pool._waitingTasksMutex);
		if (!pool._waitingTasks.empty())
		{
			thread_task* task = std::move(pool._waitingTasks.front());
			pool._waitingTasks.pop();
			pool._threadsTaskAwaible = !pool._waitingTasks.empty();
			return task;
		}
		return nullptr;
	}

	static void addCompletedTask(thread_pool& pool, thread_task*& task)
	{
		std::scoped_lock<std::mutex> lock(pool._completedTasksMutex);
		pool._completedTasks.push(task);
	}
};

static void thread_loop_func(void* data)
{
	thread_pool& pool = *reinterpret_cast<thread_pool*>(data);
	while (pool.getThreadLoopCondition())
	{
		if (pool.getThreadTaskAvaible())
		{
			thread_task* task = thread_pool_accessor::tryPopTask(pool);
			if (task)
			{
				task->doJob();
				if (task->useCompleted())
				{
					thread_pool_accessor::addCompletedTask(pool, task);
				}
				else 
				{
					delete task;
				}
			}
		}
		else 
		{
			std::this_thread::yield();
		}
	}
}

thread_pool::thread_pool()
	: _totalTaskCount{0}
{
	const auto num = std::thread::hardware_concurrency() / 2;

	_threads.resize(num);
	for (auto i = 0U; i < num; ++i)
	{
		_threads[i] = std::thread(thread_loop_func, this);
	}
}

thread_pool::~thread_pool()
{
	_threadsLoopCondition = false;
	for (auto& thread : _threads)
	{
		thread.join();
	}

	while(!_waitingTasks.empty())
	{
		thread_task* task = std::move(_waitingTasks.front());
		delete task;
		_waitingTasks.pop();
	}
	processCompletedTasks();
}

void thread_pool::tick()
{
	processCompletedTasks();
}

void thread_pool::queueTask(thread_task* task)
{
	task->_id = ++_totalTaskCount;

	std::scoped_lock<std::mutex> guard(_waitingTasksMutex);
	_waitingTasks.push(task);
	task->init();
	task->markStart();

	_threadsTaskAwaible = true;
}

void thread_pool::processCompletedTasks()
{
	while (!_completedTasks.empty())
	{
		std::scoped_lock<std::mutex> lock(_completedTasksMutex);
		thread_task* task = std::move(_completedTasks.front());
		_completedTasks.pop();

		task->compeleted();
		task->markEnd();

		std::cout << "thread_pool: completed task with id: " << task->getId() << "; took : " << task->getTaskCompletionTime() << "s" << std::endl;

		delete task;
	}
}