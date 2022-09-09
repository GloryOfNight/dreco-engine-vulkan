#pragma once

#ifdef __linux__
#define PLATFORM_LINUX 1
#endif

#ifdef __WIN32__
#define PLATFORM_WINDOWS 1
#endif

#include "platforms/paths/generic.hxx"
using platform_paths = generic_paths;