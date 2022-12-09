#pragma once

#include <SDL_log.h>
#include <string>

namespace de
{
	enum class log_category
	{
		Application = SDL_LOG_CATEGORY_APPLICATION,
		Error = SDL_LOG_CATEGORY_ERROR,
		Assert = SDL_LOG_CATEGORY_ASSERT,
		System = SDL_LOG_CATEGORY_SYSTEM,
		Audio = SDL_LOG_CATEGORY_AUDIO,
		Video = SDL_LOG_CATEGORY_VIDEO,
		Render = SDL_LOG_CATEGORY_RENDER,
		Input = SDL_LOG_CATEGORY_INPUT,
		Test = SDL_LOG_CATEGORY_TEST,
	};
	enum class log_priority
	{
		Verbose = SDL_LOG_PRIORITY_VERBOSE,
		Debug = SDL_LOG_PRIORITY_DEBUG,
		Info = SDL_LOG_PRIORITY_INFO,
		Warn = SDL_LOG_PRIORITY_WARN,
		Error = SDL_LOG_PRIORITY_ERROR,
		Critical = SDL_LOG_PRIORITY_CRITICAL
	};

	template <typename... Args>
	static void log(log_category category, log_priority priority, const std::string_view message, Args&&... args) noexcept
	{
		SDL_LogMessage(static_cast<SDL_LogCategory>(category), static_cast<SDL_LogPriority>(priority), message.data(), std::forward<Args>(args)...);
	}
} // namespace de

#define DE_LOG(priority, message, ...) de::log(de::log_category::Application, de::log_priority::priority, message, ##__VA_ARGS__)
#define DE_LOGC(category, priority, message, ...) de::log(de::log_category::category, de::log_priority::priority, message, ##__VA_ARGS__)