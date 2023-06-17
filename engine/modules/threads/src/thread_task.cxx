#include "thread_task.hxx"

void de::async::thread_task::init()
{
	markStart();
}

void de::async::thread_task::completed()
{
	for (const auto& callback : _callbacks)
	{
		try
		{
			std::invoke(callback, this);
		}
		catch (std::bad_function_call badFuncCall)
		{
			DE_LOG(Error, "%s: caught exception bad_function_call: %s", __FUNCTION__, badFuncCall.what());
		}
	}
	markEnd();

	DE_LOG(Info, "%s: completed async task with id: %u, took: %fs", __FUNCTION__, static_cast<unsigned int>(getId()), getTaskCompletionTime());
}

void de::async::thread_task::bindCallback(callback&& inCallback)
{
	_callbacks.emplace_back(inCallback);
}

void de::async::thread_task::unbindAll()
{
	_callbacks.clear();
}

uint64_t de::async::thread_task::getId() const
{
	return _id;
}

double de::async::thread_task::getTaskCompletionTime() const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(_end - _begin).count() / 1000.0;
};

void de::async::thread_task::abort()
{
	_abort = true;
}

bool de::async::thread_task::isAborted() const
{
	return _abort;
}

void de::async::thread_task::markStart()
{
	_begin = std::chrono::steady_clock::now();
};

void de::async::thread_task::markEnd()
{
	_end = std::chrono::steady_clock::now();
};
