#pragma once

#include "dreco.hxx"

#include <chrono>
#include <functional>
#include <memory>
#include <atomic>

struct thread_task
{
protected:
	thread_task() = default;

public:
	using shared = std::shared_ptr<thread_task>;
	using callback = std::function<void(thread_task*)>;

	virtual ~thread_task() = default;

	virtual void init();

	virtual void doJob() = 0;

	virtual void completed();

	template <typename Task, class... Args>
	static Task* makeNew(uint64_t id, Args&&... args);

	template <typename T, typename F>
	void bindCallback(T* obj, F func);

	void bindCallback(callback&& inCallback);

	uint64_t getId() const;

	void abort();
	bool isAborted() const;

	double getTaskCompletionTime() const;

private:
	void markStart();
	void markEnd();

	uint64_t _id{UINT64_MAX};

	std::atomic<bool> _abort{false};

	std::chrono::steady_clock::time_point _begin{};
	std::chrono::steady_clock::time_point _end{};

	std::vector<callback> _callbacks{};
};

template <typename Task, class... Args>
Task* thread_task::makeNew(uint64_t id, Args&&... args)
{
	auto task = new Task(std::forward<Args>(args)...);
	task->_id = id;
	return task;
}

template <typename T, typename F>
void thread_task::bindCallback(T* obj, F func)
{
	bindCallback(std::bind(func, obj, std::placeholders::_1));
}