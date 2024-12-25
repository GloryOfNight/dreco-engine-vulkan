#pragma once

#include <SDL3/SDL_platform.h>

#define PLATFORM_LINUX __LINUX__
#define PLATFORM_WINDOWS _WIN32

#include "paths/generic.hxx"

namespace de::platform
{
	using path = de::paths::generic;
} // namespace de::platform
