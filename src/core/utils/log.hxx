#pragma once

#include <SDL.h>

#define DE_LOG(priority, message, ...) SDL_Log##priority(SDL_LOG_CATEGORY_APPLICATION, message, ##__VA_ARGS__)