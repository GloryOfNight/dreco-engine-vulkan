#pragma once

#include <SDL3/SDL_log.h>
#include <string>
#include <type_traits>

namespace de
{
	namespace log
	{
		enum class category
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
		enum class priority
		{
			Verbose = SDL_LOG_PRIORITY_VERBOSE,
			Debug = SDL_LOG_PRIORITY_DEBUG,
			Info = SDL_LOG_PRIORITY_INFO,
			Warn = SDL_LOG_PRIORITY_WARN,
			Error = SDL_LOG_PRIORITY_ERROR,
			Critical = SDL_LOG_PRIORITY_CRITICAL
		};

		template <typename... Args>
		static void message(category category_, priority priority_, const std::string_view format, Args&&... args)
		{
			if constexpr (sizeof...(args) == 0)
			{
				SDL_LogMessage(static_cast<SDL_LogCategory>(category_), static_cast<SDL_LogPriority>(priority_), "%s", format.data());
			}
			else
			{
				SDL_LogMessage(static_cast<SDL_LogCategory>(category_), static_cast<SDL_LogPriority>(priority_), format.data(), std::forward<Args>(args)...);
			}
		}
	} // namespace log
} // namespace de

#define DE_LOG(priority_, format, ...) de::log::message(de::log::category::Application, de::log::priority::priority_, format, ##__VA_ARGS__)
#define DE_LOGC(category_, priority_, format, ...) de::log::message(de::log::category::category_, de::log::priority::priority_, format, ##__VA_ARGS__)