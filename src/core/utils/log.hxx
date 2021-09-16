#pragma once

#include <SDL_log.h>
#include <cstdarg>
#include <string>

#define DR_LOGF(priority, message, ...) DR_LOG(priority, message, ##__VA_ARGS__)
#define DR_LOG(priority, message, ...) de::log(de::log_priority::priority, message, ##__VA_ARGS__)

namespace de
{
	enum class log_priority
	{
		Verbosity = SDL_LOG_PRIORITY_VERBOSE,
		Debug = SDL_LOG_PRIORITY_DEBUG,
		Info = SDL_LOG_PRIORITY_INFO,
		Warning = SDL_LOG_PRIORITY_WARN,
		Error = SDL_LOG_PRIORITY_ERROR,
		Critical = SDL_LOG_PRIORITY_CRITICAL
	};

	static void log(const log_priority priority, const std::string_view& message, ...)
	{
		va_list args;
		va_start(args, 0);
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, static_cast<SDL_LogPriority>(priority), message.data(), args);
	}
} // namespace de