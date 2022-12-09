#pragma once

#include "SDL_platform.h"

#define PLATFORM_LINUX __LINUX__
#define PLATFORM_WINDOWS __WINDOWS__

#include "paths/generic.hxx"

namespace de::platform
{
	using path = de::paths::generic;
} // namespace de::platform
