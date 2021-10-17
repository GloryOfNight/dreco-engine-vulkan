#pragma once

#include <SDL_log.h>
#include <cstdarg>

namespace de
{
	enum class log_priority : uint8_t
	{
		Verbose = SDL_LOG_PRIORITY_VERBOSE,
		Debug = SDL_LOG_PRIORITY_DEBUG,
		Info = SDL_LOG_PRIORITY_INFO,
		Warn = SDL_LOG_PRIORITY_WARN,
		Error = SDL_LOG_PRIORITY_ERROR,
		Critical = SDL_LOG_PRIORITY_CRITICAL
	};
	static void log(log_priority priority, const char* fmt, ...)
	{
		va_list list;
		va_start(list, fmt);
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, static_cast<SDL_LogPriority>(priority), fmt, list);
		va_end(list);
	}
} // namespace de

#define DE_LOG(priority, message, ...) de::log(de::log_priority::##priority, message, ##__VA_ARGS__)