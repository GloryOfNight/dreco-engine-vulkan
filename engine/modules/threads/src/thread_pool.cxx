#include "thread_pool.hxx"

#include "SDL_thread.h"

#include <algorithm>

de::async::thread_pool::~thread_pool()
{
	freeThreads();
}

void de::async::thread_pool::allocateThreads(const std::string_view name, const uint32_t threadCount, const priority priority)
{
	_threads.reserve(threadCount);
	for (uint32_t i = 0; i < threadCount; ++i)
	{
		_threads.emplace_back(SDL_CreateThread(thread_pool::threadsLoop, name.data(), this));
	}
	DE_LOG(Verbose, "%s: allocated %i threads", __FUNCTION__, _threads.size());
}

void de::async::thread_pool::freeThreads()
{
	if (_threads.size() == 0)
		return;

	_loopCondition = false;
	for (auto thread : _threads)
	{
		int status{0};
		SDL_WaitThread(thread, &status);
		DE_LOG(Verbose, "%s: thread %p exit with code: %i", __FUNCTION__, thread, status);
	}
	_loopCondition = true;
	_threads.clear();
}

void de::async::thread_pool::setCleanupFrame(uint64_t inValue)
{
	_cleanupFrame = inValue;
}

bool de::async::thread_pool::getLoopCondition() const
{
	return _loopCondition;
};

de::async::thread_pool::priority de::async::thread_pool::getPriority() const
{
	return _priority;
}

void de::async::thread_pool::tick(uint64_t frameCount)
{
	const auto processTasks = [](auto& pair)
	{
		if (pair.second == task_state::uninitialized)
		{
			pair.first->init();
			pair.second = task_state::waiting;
		}
		else if (pair.second == task_state::processed)
		{
			pair.second = task_state::done;
			pair.first->completed();
		}
	};
	std::for_each(_tasks.begin(), _tasks.end(), processTasks);

	if (frameCount % _cleanupFrame == 0)
	{
		std::scoped_lock<std::mutex> lock(_tasksMutex);
		while (1)
		{
			auto it = std::find_if(_tasks.begin(), _tasks.end(),
				[](auto& pair) -> bool
				{
					return pair.second == task_state::done;
				});
			if (it == _tasks.end())
				break;
			_tasks.erase(it);
		}
		DE_LOG(Verbose, "%s: cleanup done", __FUNCTION__);
	}
}

uint32_t de::async::thread_pool::hardwareConcurrency()
{
	return static_cast<uint32_t>(std::thread::hardware_concurrency());
}

uint64_t de::async::thread_pool::makeTaskId()
{
	return ++_totalTaskCount;
}

de::async::thread_task::shared de::async::thread_pool::findTask(const uint64_t taskId)
{
	auto it = std::find_if(_tasks.begin(), _tasks.end(),
		[taskId](const auto& pair) -> bool
		{
			return pair.first->getId() == taskId;
		});
	return it != _tasks.end() ? it->first : thread_task::shared();
}

int de::async::thread_pool::threadsLoop(void* data)
{
	auto& pool = *reinterpret_cast<thread_pool*>(data);

	SDL_SetThreadPriority(static_cast<SDL_ThreadPriority>(pool.getPriority()));

	while (pool.getLoopCondition())
	{
		if (auto task = pool.beginTaskProcessing())
		{
			if (task)
			{
				if (!task->isAborted())
					task->doJob();
				pool.endProcessingTask(task);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3));
		}
	}
	return 0;
}

de::async::thread_task* de::async::thread_pool::beginTaskProcessing()
{
	std::scoped_lock<std::mutex> lock(_tasksMutex);
	auto it = std::find_if(_tasks.begin(), _tasks.end(),
		[](auto& pair) -> bool
		{
			return pair.second == task_state::waiting;
		});
	if (it != _tasks.end())
	{
		it->second = task_state::procesing;
		return it->first.get();
	}
	return nullptr;
}

void de::async::thread_pool::endProcessingTask(thread_task* task)
{
	std::scoped_lock<std::mutex> lock(_tasksMutex);
	auto it = std::find_if(_tasks.begin(), _tasks.end(),
		[task](const auto& pair) -> bool
		{
			return pair.first.get() == task;
		});
	if (it != _tasks.end())
		it->second = task_state::processed;
}