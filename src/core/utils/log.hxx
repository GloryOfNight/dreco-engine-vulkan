#pragma once

#include <SDL.h>
#include <string_view>

#define DR_LOGF(category, message, ...) DR_LOG(category, (std::string(__FUNCTION__) + ": " + message).data(), __VA_ARGS__)
#define DR_LOG(category, message, ...) SDL_Log##category(SDL_LOG_CATEGORY_APPLICATION, message, __VA_ARGS__)